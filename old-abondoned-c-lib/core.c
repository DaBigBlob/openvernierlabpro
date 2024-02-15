/* liblabpro, a C library for using Vernier LabPro devices.
 * Based on the original FreeLab Ruby implementation by Ben Crowell.
 * 
 * * www.lightandmatter.com/freelab
 * * liblabpro.sf.net
 * 
 * Copyright (C) 2018 Matthew Trescott <matthewtrescott@gmail.com>
 * 
 * liblabpro is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * liblabpro is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with liblabpro.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "backends/labpro/labpro-internal.h"
#include <libusb-1.0/libusb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef WIN32
#include <windows.h>
#endif
#ifdef __unix__
#include <time.h>
#endif

#ifndef NDEBUG
#define DEBUG
#endif

void LabPro_sleep(unsigned int milliseconds) {
#ifdef WIN32
    Sleep(milliseconds);
#endif
#ifdef __unix__
    struct timespec spec;
    spec.tv_sec = 0;
    spec.tv_nsec = milliseconds * 1000000;
    nanosleep(&spec, NULL);
#endif
}

int LabPro_init(LabPro_Context *context) {
    int errorcode = libusb_init(&(*context).usb_link);
    if (errorcode != LIBUSB_SUCCESS) {
        printf("[liblabpro FATAL] Error initializing liblabpro: %s\n", libusb_strerror(errorcode));
    }
#ifdef DEBUG
    libusb_set_debug((*context).usb_link, LIBUSB_LOG_LEVEL_DEBUG);
#else
    libusb_set_debug((*context).usb_link, LIBUSB_LOG_LEVEL_WARNING);
#endif
    return errorcode;
}

void LabPro_exit(LabPro_Context* context) {
    libusb_exit((*context).usb_link);
}

LabPro_List LabPro_list_labpros(LabPro_Context* context) {
    libusb_device** usb_list;
    LabPro_List lp_list;
    ssize_t cnt;
    
    cnt = libusb_get_device_list((*context).usb_link, &usb_list);
    if (cnt <= 0 ) {
        lp_list.num = 0;
    }
    else {
        lp_list.num = 0;
        struct libusb_device_descriptor desc;
        
        for (ssize_t i = 0; i < cnt && lp_list.num < 5; ++i) { // We won't support more than 5 connected LabPros at once, that would be ridiculous
            libusb_get_device_descriptor(usb_list[i], &desc);
            if (desc.idVendor == 0x08f7 && desc.idProduct == 1) {
                // Attempt to open the device
                libusb_device_handle *dev_handle = NULL;
                int open_err;
                if ((open_err = libusb_open(usb_list[i], &dev_handle)) != LIBUSB_SUCCESS) {
                    printf("[liblabpro ERR] Unable to open libusb device: %s\n", libusb_strerror(open_err));
                }
                else {
                    // Detach kernel driver if necessary
                    if (libusb_kernel_driver_active(dev_handle, 0) == 1) {
                        int detach_error = libusb_detach_kernel_driver(dev_handle, 0);
                        if (detach_error != 0) {
                            printf("[liblabpro ERR] Unable to detach kernel driver from interface 0 of LabPro %d: %s\n", lp_list.num, libusb_strerror(detach_error));
                            libusb_close(dev_handle);
                            continue;
                        }
                        else {
                            printf("[liblabpro MSG] Successfully detached kernel driver.\n");
                        }
                    }
                    
                    /* I don't think that LabPros have more than one USB configuration.
                     * freelab doesn't manually set the configuration probably because the
                     * kernel sometimes chooses a configuration automatically.
                     */
                    int config_setting_err = libusb_set_configuration(dev_handle, 1);
                    if (config_setting_err != 0) {
                        printf("[liblabpro ERR] Unable to set configuration to 1 on LabPro %d: %s\n", lp_list.num, libusb_strerror(config_setting_err));
                        libusb_close(dev_handle);
                        continue;
                    }
                    
                    // Claim the interface in order to be able to write to endpoints.
                    int interface_claim_error;
                    if ((interface_claim_error = libusb_claim_interface(dev_handle, 0)) != 0) {
                        printf("[liblabpro ERR] Unable to claim interface 0 of LabPro %d: %s\n", lp_list.num, libusb_strerror(interface_claim_error));
                        libusb_close(dev_handle);
                        continue;
                    }
                    
                    // Find endpoints
                    struct libusb_config_descriptor* config;
                    int config_desc_err;
                    if ((config_desc_err = libusb_get_config_descriptor(usb_list[i], 0, &config)) != 0) {
                        printf("[liblabpro ERR] Unable to get descriptor of first configuration of LabPro %d: %s\n", lp_list.num, libusb_strerror(config_desc_err));
                        libusb_close(dev_handle);
                        continue;
                    }
                    struct libusb_endpoint_descriptor ep_desc;
                    unsigned char in_addr;
                    unsigned char out_addr;
                    bool in_endpt_found = false;
                    bool out_endpt_found = false;
                    for (uint8_t i = 0; i < config->interface[0].altsetting[0].bNumEndpoints; ++i) {
                        ep_desc = config->interface[0].altsetting[0].endpoint[i];
#ifdef DEBUG
                        printf("[liblabpro DBG] EP Address: %x, Endpoint attributes: %x\n", (unsigned int)ep_desc.bEndpointAddress, (unsigned int)ep_desc.bmAttributes);
#endif
                        // Make sure we're dealing with only bulk endpoints.
                        if ((ep_desc.bmAttributes & 0b00000011) != LIBUSB_TRANSFER_TYPE_BULK)
                        {
                            printf("[liblabpro ERR] LabPro %d had unexpected non-bulk endpoint (Endpoint attributes: %x).\n", lp_list.num, (unsigned int)ep_desc.bmAttributes);
                            continue;
                        }
                        
                        /* I'm not sure whether this would fail on big-endian machines or not. The libusb docs
                         * are not clear about whether the descriptor values get converted to big-endian on
                         * big-endian systems, but I do not think they do.
                         * Additionally, the docs talk about "bits 0:3 and 4:6" but don't specify whether bit 0
                         * is the LSB or MSB. Based on freelab, I have to assume the former.
                         */
                        if ((ep_desc.bEndpointAddress & 0b10000000) == LIBUSB_ENDPOINT_IN) {
                            in_addr = ep_desc.bEndpointAddress;
                            in_endpt_found = true;
#ifdef DEBUG
                            printf("[liblabpro DBG] Using endpoint %x as bulk in endpoint.\n", (unsigned int)in_addr);
#endif
                        }
                        else {
                            out_addr = ep_desc.bEndpointAddress;
                            out_endpt_found = true;
#ifdef DEBUG
                            printf("[liblabpro DBG] Using endpoint %x as bulk out endpoint.\n", (unsigned int)out_addr);
#endif
                        }
                    }
                    libusb_free_config_descriptor(config);
                    
                    if (!in_endpt_found || !out_endpt_found) {
                        printf("[liblabpro ERR] Bulk endpoints not found for LabPro %d.\n", lp_list.num);
                        libusb_release_interface(dev_handle, 0);
                        libusb_close(dev_handle);
                        continue;
                    }
                    
                    
                    LabPro *labpro = (LabPro*)malloc(sizeof(LabPro));
                    labpro->device_handle = dev_handle;
                    labpro->is_open = true;
                    labpro->in_endpt_addr = in_addr;
                    labpro->out_endpt_addr = out_addr;
                    labpro->timeout = 5000; // This is what freelab uses
                    lp_list.labpros[lp_list.num] = labpro;
                    ++lp_list.num;
                }
            }
        }
    }
    libusb_free_device_list(usb_list, 1);
    return lp_list;
}

void LabPro_close_labpro(LabPro* labpro) {
    libusb_release_interface(labpro->device_handle, 0);
    libusb_close(labpro->device_handle);
    labpro->is_open = false;
}

int LabPro_reset(LabPro* labpro, bool force) {
    if (!labpro->is_open)
        return LABPRO_ERR_NOT_OPEN;
    
    if (labpro->is_busy && !force)
        return LABPRO_ERR_BUSY;
    
    if (labpro->is_collecting_data && !force)
        return LABPRO_ERR_BUSY_COLLECT;
    
    unsigned char *cmd_str = (unsigned char *)malloc(6);
    snprintf((char*)cmd_str, 6, "s{%d}\r", LABPRO_RESET);
    int transferred;
    
    
    return LabPro_send_raw(labpro, (char*)cmd_str, &transferred);
}

int LabPro_check_data_session(LabPro_Data_Session* session, int** errors) {
    int count = 0;
    int *found_errors = (int *)calloc(5, sizeof(int));
    
    if ((session->analog_op != 0 && session->channel > 4) || (session->sonic_op != 0 && session->channel <= 4)) {
        found_errors[count] = LABPRO_ERR_OP_MISMATCH;
        count++;
    }
    if (session->postproc != 0 && session->sampling_mode == LABPRO_SAMPMODE_REALTIME) {
        found_errors[count] = LABPRO_ERR_POSTPROC_ON_REALTIME;
        count++;
    }
    if (session->postproc != 0 && session->channel > 4) {
        found_errors[count] = LABPRO_ERR_POSTPROC_ON_SONIC;
        count++;
    }
    
    *errors = found_errors;
    return count;
}

int LabPro_send_raw(LabPro* labpro, char* command, int* length_transferred) {
    *length_transferred = 0;
    if (!labpro->is_open)
        return LABPRO_ERR_NOT_OPEN;
    
    char* real_command = malloc(strlen(command) + 2);
    if (real_command == NULL)
        return LABPRO_ERR_NO_MEM;
    strcpy(real_command, command);
    strcat(real_command, "\r");
    
    int len = strlen(real_command);
    int numbytes; // How many bytes to transfer
    int transferred; // Number of bytes actually transferred, as reported by libusb
    int status; // Return value from libusb_bulk_transfer()
    int numerrors = 0; // How many times libusb has returned an error
    int numpackets = len / 64;
    if (len % 64 > 0)
        ++numpackets;
    
    for (int i = 1; i <= numpackets; ++i) {
        LabPro_sleep(50);
        
        if (i == numpackets && len % 64 != 0)
            numbytes = len % 64;
        else
            numbytes = 64;
        
        status = libusb_bulk_transfer(
            labpro->device_handle,
            labpro->out_endpt_addr,
            (unsigned char*)real_command + (64 * (i - 1)),
            numbytes,
            &transferred,
            labpro->timeout
        );
        *length_transferred += transferred;
        
        if (status == LIBUSB_ERROR_NO_DEVICE) {
            LabPro_handle_device_disconnect(labpro);
            return LIBUSB_ERROR_NO_DEVICE;
        }
        
        if (status != LIBUSB_SUCCESS) {
            ++numerrors;
            printf("[liblabpro ERR] Error writing to USB: %s\n", libusb_strerror(status));
            printf("[liblabpro WARN] There have been %d errors for this write function so far.\n", numerrors);
            --i;
            
            if (numerrors > 5) {
                printf("[liblabpro ERR] LabPro_send_raw: Error limit reached; aborting.");
                return status;
            }
        }
    }
    
    free(real_command);
    return LABPRO_OK;
}

int LabPro_read_raw(LabPro* labpro, char** string, int* length) {
    *length = 0;
    if (!labpro->is_open)
        return LABPRO_ERR_NOT_OPEN;
    
    int retval = LABPRO_OK;
    int status; // The status code libusb_bulk_transfer returns
    int transferred; // Number of bytes transferred. This should always be either 0 or 64
    int numpackets = 1; // Number of packets we need the buffer to be able to store.
    int numerrors = 0; // Number of times libusb has returned an error
    unsigned char* data = calloc(64, sizeof(char)); // The data buffer
    if (data == NULL)
        return LABPRO_ERR_NO_MEM;
    
    do {
        LabPro_sleep(50);
        status = libusb_bulk_transfer(
            labpro->device_handle,
            labpro->in_endpt_addr,
            data + (64 * (numpackets - 1)), // Write into the proper offset
            64,
            &transferred,
            labpro->timeout
        );
        
        if (status == LIBUSB_ERROR_NO_DEVICE) {
            LabPro_handle_device_disconnect(labpro);
            memset(data + (64 * (numpackets - 1)), 0, 64); // NULL-terminate the string since we allocated this memory and didn't fill it.
            retval = LIBUSB_ERROR_NO_DEVICE;
            break;
        }
        
        if (status != LIBUSB_SUCCESS && status != LIBUSB_ERROR_TIMEOUT) {
            ++numerrors; // We do not need to check transferred because the transfer is atomic in this case. https://sourceforge.net/p/libusb/mailman/message/36289834/
            printf("[liblabpro ERR] Error reading from USB: %s\n", libusb_strerror(status));
            printf("[liblabpro WARN] There have been %d errors for this read function call so far.\n", numerrors);
            
            if (numerrors > 5) {
                printf("[liblabpro ERR] LabPro_read_raw: error limit reached; aborting.\n");
                memset(data + (64 * (numpackets - 1)), 0, 64); // NULL-terminate the string since we allocated this memory and didn't fill it.
                retval = status;
                break;
            }
            continue;
        }
        else if (status == LIBUSB_ERROR_TIMEOUT) { // There is no more data to read; so we can return
            memset(data + (64 * (numpackets - 1)), 0, 64); // NULL-terminate the string since we allocated this memory and didn't fill it.
            break;
        }
        
        // This only gets executed on successful reads
        *length += transferred;
        ++numpackets;
        
        unsigned char* previous_data = data;
        data = realloc(data, 64 * numpackets); // Allocate memory for the next read.
        if (data == NULL) {
            data = previous_data;
            retval = LABPRO_ERR_NO_MEM;
            break;
        }
        
    } while (transferred == 64 && status == LIBUSB_SUCCESS);
    
    *string = (char*)data;
    
    return retval;
}

int LabPro_trim_response(char* string) {
    char* cr = strstr(string, "\r");
    if (cr == NULL)
        return 1; // Not found
    else
        *cr = '\0';
    return 0;
}

int LabPro_parse_list(char* string, int* argc_list, char ***argv_list)
{
    *argc_list = 0;
    if (string[0] != '{' || strstr(string, "}") == NULL)
        return LABPRO_ERR_BAD_LIST;
    
    string = strstr(string, "{") + 1;
    *argv_list = malloc(1 * sizeof(char*));
    if (*argv_list == NULL)
        return LABPRO_ERR_NO_MEM;
    
    int element_length;
    bool return_after_copy = false;
    for (int i = 0; true; ++i) {
        if (strstr(string, ",") != NULL) {
            element_length = (uintptr_t)strstr(string, ",") - (uintptr_t)string;
        }
        else {
            if (strstr(string, "}") == NULL)
                return LABPRO_ERR_BAD_LIST;
            
            element_length = (uintptr_t)strstr(string, "}") - (uintptr_t)string;
            return_after_copy = true;
        }
        
        (*argv_list)[i] = calloc((element_length + 1), sizeof(char));
        if ((*argv_list)[i] == NULL)
            return LABPRO_ERR_NO_MEM;
        
        memcpy((*argv_list)[i], string, element_length);
        
        if (return_after_copy) {
            *argc_list = i + 1;
            return LABPRO_OK;
        }
        
        char** argv_new = realloc(*argv_list, (i * sizeof(char*)) + (2 * sizeof(char*)));
        if (argv_new == NULL)
            return LABPRO_ERR_NO_MEM;
        else
            *argv_list = argv_new;
        
        string = strstr(string, ",") + 1;
    }
    return LABPRO_OK;
}

int LabPro_query_status(LabPro* labpro) {
    if (labpro->is_fastmode_running) {
#ifdef DEBUG
        printf("[liblabpro DBG] Waiting for FastMode to complete.\n");
#endif
    }
    
    while (labpro->is_fastmode_running) {
        LabPro_sleep(1);
    }
    
    char command[4];
    sprintf(command, "{%d}", LABPRO_SYS_STATUS);
    int transferred;
    LabPro_send_raw(labpro, command, &transferred);
    
    return LABPRO_OK;
}


void LabPro_handle_device_disconnect(LabPro* labpro) {
    printf("[liblabpro WARN] LabPro_handle_device_disconnect(): stub\n");
    return;
}
