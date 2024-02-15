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
 * 
 */

/** \file
 * \defgroup init_deinit Initialization/Deinitialization
 * \defgroup labpro_interface Features specific to the LabPro itself (not sensors)
 * \defgroup data_collection Data Collection
 * \defgroup misc Miscellaneous
 * \defgroup internal Internal functions, enums, etc.
 */

/** \mainpage liblabpro API Reference
 * 
 * Introduction
 * ------------
 * liblabpro is an open-source library that allows you to collect data from Vernier LabPro sensor interfaces.
 * Special thanks to Vernier for publishing the [LabPro Technical Reference Manual](http://www2.vernier.com/labpro/labpro_tech_manual.pdf) and Ben Crowell for his additional reverse engineering on [FreeLab](http://lightandmatter.com/freelab/).
 * 
 * Overview
 * --------
 * See the Modules page for more details, but here are the basic steps:
 * 
 * - Initialize with LabPro_init()
 * - Search for LabPro interfaces with LabPro_list_labpros()
 * - Choose the LabPro you want to use and run LabPro_close_labpro() on all the others.
 * - Reset the device with LabPro_reset()
 * - Fill a LabPro_Data_session struct with parameters, then verify that it will work
 *   with LabPro_check_data_session().
 * - To be continued...
 */

#pragma once
#include <stdbool.h>
#include <libusb-1.0/libusb.h>

/** \brief Commands that can be sent to the LabPro
 * \ingroup internal
 */
enum LabPro_Commands {
    /** \brief Clear RAM and reset the LabPro */
    LABPRO_RESET                = 0,
    
    /** \brief Select channels and units for data collection. */
    LABPRO_CHANNEL_SETUP        = 1,
    
    /** \brief Set data collection rate and time, etc. */
    LABPRO_DATACOLLECT_SETUP    = 3,
    
    /** \brief Set up a manual conversion equation rather than one obtained by Auto-ID. */
    LABPRO_CONVERSION_EQN_SETUP = 4,
    
    /** \brief Set parameters for what data points will be returned. */
    LABPRO_DATA_CTL             = 5,
    
    /** \brief Set various settings for the LabPro itself. */
    LABPRO_SYS_SETUP            = 6,
    LABPRO_SYS_STATUS           = 7,
    
    /** \brief Get a data point during non-realtime collection */
    LABPRO_CHAN_STATUS          = 8,
    
    /** \brief Get a single point of data outside of active data collection. */
    LABPRO_REQUEST_CHAN_DATA    = 9,
    LABPRO_ADVANCED_DATA_REDUCTION = 10,
    LABPRO_DIGITAL_DATA_CAPTURE = 12,
    
    /** Return sensor IDs for each channel */
    LABPRO_QUERY_CHANNELS       = 80,
    LABPRO_PORT_POWER_CTL       = 102,
    LABPRO_REQUEST_SETUP_INFO   = 115,
    LABPRO_REQUEST_LONG_SENSOR_NAME = 116,
    LABPRO_REQUEST_SHORT_SENSOR_NAME = 117,
    LABPRO_ARCHIVE              = 201,
    LABPRO_ANALOG_OUT_SETUP     = 401,
    LABPRO_LED_CTL              = 1998,
    LABPRO_SOUND_CTL            = 1999,
    LABPRO_DIGITAL_OUT_CTL      = 2001
};

/** \brief Battery levels reported by the LabPro
 * \ingroup labpro_interface
 */
enum LabPro_Battery_Level {
    LABPRO_BATTERY_OK,
    LABPRO_BATTERY_LOW_WHILE_SAMPLING,
    LABPRO_BATTERY_LOW
};

/** \brief Channel ports on the LabPro interface
 * \ingroup data_collection
 */
enum LabPro_Channels {
    /** \brief Used when e.g. resetting all channels at once. */
    LABPRO_CHAN_ALL             = 0,
    LABPRO_CHAN_ANALOG_1        = 1,
    LABPRO_CHAN_ANALOG_2        = 2,
    LABPRO_CHAN_ANALOG_3        = 3,
    LABPRO_CHAN_ANALOG_4        = 4,
    LABPRO_CHAN_SONIC_1         = 11,
    LABPRO_CHAN_SONIC_2         = 12,
    LABPRO_CHAN_DIGITAL_1       = 21,
    LABPRO_CHAN_DIGITAL_2       = 22,
    LABPRO_CHAN_DIGITAL_OUT_1   = 31,
    LABPRO_CHAN_DIGITAL_OUT_2   = 32
};

/** \brief Operations for analog channels
 * \ingroup internal
 */
enum LabPro_Analog_Chan_Operations {
    /** \brief Turn the channel off. */
    LABPRO_CHANOP_OFF,
    /** \brief Auto-ID the sensor; units can be fetched from the sensor's description. */
    LABPRO_CHANOP_AUTOID,
    /** \brief Range from -10 V to +10 V. */
    LABPRO_CHANOP_VOLTAGE10V, // Range from -10V to +10V. Same for period, frequency, and transitions ops below.
    /** \brief Presumably 0 Amps to 10 Amps */
    LABPRO_CHANOP_CURRENT10A,
    /** \brief Measure resistance. */
    LABPRO_CHANOP_RESISTANCE,
    /** \brief Measure the signal period on a ±10 V signal. Channel 1 only. */
    LABPRO_CHANOP_VOLTAGE10V_PERIOD,
    /** \brief Measure the signal frequency on a ±10 V signal. Channel 1 only. */
    LABPRO_CHANOP_VOLTAGE10V_FREQUENCY,
    /** \brief Count signal transitions on a ±10 V signal. Channel 1 only. */
    LABPRO_CHANOP_VOLTAGE10V_TRANSITIONS_COUNT,
    /** \brief Measure temperature (Celsius) from the TI Temperature probe
     * This is for measuring temperature in Celsius from the Texas Instruments Stainless
     * Steel Temperature Probe. The CBL2 technical reference manual states that this probe
     * supports Auto-ID, so I'm not sure what the purpose of this setting is. Maybe early
     * revisions of the sensor didn't auto-ID...?
     */
    LABPRO_CHANOP_TI_TEMP_C = 10,
    /** \brief Measure temperature (Fahrenheit) from the TI Temperature probe
     * This is for measuring temperature in Fahrenheit from the Texas Instruments Stainless
     * Steel Temperature Probe. The CBL2 technical reference manual states that this probe
     * supports Auto-ID, so I'm not sure what the purpose of this setting is. Maybe early
     * revisions of the sensor didn't auto-ID...?
     */
    LABPRO_CHANOP_TI_TEMP_F = 11,
    /** \brief Measure light from the TI Light Sensor
     * This is for the Texas Instruments light sensor. (I think) it causes the LabPro to
     * return units of milliwatts per square centimeter, since that is what the CBL2 technical
     * manual says its units are. However, it also states that this sensor Auto-IDs, so I'm
     * not sure this is necessary.
     */
    LABPRO_CHANOP_TI_LIGHT = 12,
    /** \brief Higher-precision voltage measurement
     * I assume this is just like the ±10 V measurement, but more precise due to the smaller range.
     */
    LABPRO_CHANOP_VOLTAGE_ZERO_TO_FIVE = 14
};

/** \brief Operations for sonic channels
 * \ingroup internal
 */
enum LabPro_Sonic_Chan_Operations {
    LABPRO_CHANOP_RESET = 0,
    /** \brief Return distance in meters and change in time since last measurement
     * Fun fact: the manual states that sending a 2 will do the same thing.
     */
    LABPRO_DISTANCE_AND_DT_METERS = 1,
    /** \brief Return distance in feet and change in time since last measurement. */
    LABPRO_DISTANCE_AND_DT_FEET = 3,
    /** \brief Return distance in meters, velocity in m/s, and delta t.
     * If used with non-real-time mode sampling, this is the same as LABPRO_DISTANCE_AND_DT_METERS.
     * It may be tempting to use this in your application and make the LabPro do the extra work, but
     * please don't. The extra computations will mean the LabPro heats up more and loses accuracy.
     */
    LABPRO_DISTANCE_VELOCITY_AND_DT_METERS = 4,
    /** \brief See LABPRO_DISTANCE_VELOCITY_AND_DT_METERS. */
    LABPRO_DISTANCE_VELOCITY_AND_DT_FEET = 5,
    /** \brief Return distance, velocity, and acceleration in m, m/s, and m/(s^2), plus delta t.
     * If used with non-real-time mode sampling, this is the same as LABPRO_DISTANCE_AND_DT_METERS.
     * It may be tempting to use this in your application and make the LabPro do the extra work, but
     * please don't. The extra computations will mean the LabPro heats up more and loses accuracy.
     */
    LABPRO_DISTANCE_VELOCITY_ACCEL_AND_DT_METERS = 6,
    /** \brief See LABPRO_DISTANCE_VELOCITY_ACCEL_AND_DT_METERS. */
    LABPRO_DISTANCE_VELOCITY_ACCEL_AND_DT_FEET = 7
};
/** \brief Post-processing performed on analog data.
 * As with data collection from sonic channels, please calculate the derivatives in your application
 * rather than making the LabPro compute them. The extra computations will raise the temperature of
 * the LabPro and make its readings less accurate.
 * \ingroup internal
 */
enum LabPro_Analog_PostProc {
    /** \brief Perform no post-processing. This is the only acceptable value in real-time mode. */
    LABPRO_POSTPROC_NONE,
    /** \brief Calculate d/dt. Only allowed in non-real-time mode. */
    LABPRO_POSTPROC_DERIV1,
    /** \brief Calculate d/dt and d^2/dt^2. Only allowed in non-real-time mode. */
    LABPRO_POSTPROC_DERIV1_AND_2
};
/** \brief Sampling modes
 * \ingroup internal
 */
enum LabPro_Sampling_Modes {
    /** \brief Have the LabPro store all the datapoints in RAM and wait for a "get" command to transfer them. */
    LABPRO_SAMPMODE_NON_REALTIME,
    /** \brief Have the LabPro send datapoints as they're collected without storing them. */
    LABPRO_SAMPMODE_REALTIME
};
/** \brief LabPro system status
 * \ingroup labpro_interface
 */
enum LabPro_System_Status {
    /** \brief The LabPro is waiting for a command. */
    LABPRO_SYSSTATUS_IDLE       = 1,
    /** \brief The LabPro is watching for a trigger condition to start collecting data. */
    LABPRO_SYSSTATUS_ARMED      = 2,
    /** \brief The LabPro is currently collecting data */
    LABPRO_SYSSTATUS_BUSY       = 3,
    /** \brief The LabPro is waiting for a "get" command to fetch the collected data. */
    LABPRO_SYSSTATUS_DONE       = 4,
    /** \brief The LabPro's self-test is running */
    LABPRO_SYSSTATUS_SELFTEST   = 5,
    /** \brief The LabPro is initializing. */
    LABPRO_SYSSTATUS_INIT       = 99
};

/** \brief Errors related to the "front-end"
 * These errors are specific to use of the LabPro and are positive integers.
 * For underlying errors arising from USB transfer errors, see LabPro_USB_Errors.
 * \ingroup misc
 */
enum LabPro_Errors {
    /** \brief OK status. */
    LABPRO_OK,
    
    /** \brief liblabpro couldn't allocate memory. */
    LABPRO_ERR_NO_MEM,
    
    /** \brief The underlying libusb_device is not open. */
    LABPRO_ERR_NOT_OPEN,
    
    /** \brief The LabPro is transferring data,
     * so we cannot send a command that requires a response from the LabPro.
     */
    LABPRO_ERR_BUSY,
    
    /** The LabPro is performing a regular-speed data collection that we
     * might not want to interrupt with a reset command.
     */
    LABPRO_ERR_BUSY_COLLECT,
    
    /** The LabPro is operating in FastMode, where it samples at speeds as
     * low as 20µs between samples. If we send any commands it will cancel
     * data collection.
     */
    LABPRO_ERR_BUSY_FASTMODE,
    
    /** The data session has a sonic or digital channel selected and
     * LabPro_Data_Session.analog_op is nonzero, or an analog channel
     * is selected and LabPro_Data_Session.sonic_op is nonzero.
     */
    LABPRO_ERR_OP_MISMATCH,
    
    /** \brief The post-processing was set to a nonzero value for a sonic data capture.
     * For sonic data like from motion detectors, you have to choose to perform
     * d/dt or d^2/(dt)^2 by the channel operation.
     */
    LABPRO_ERR_POSTPROC_ON_SONIC,
    
    /** \brief The post-processing was set to a nonzero value on a realtime capture.
     * This is not allowed.
     */
    LABPRO_ERR_POSTPROC_ON_REALTIME
};

/** \brief Some errorcodes from libusb
 * These are error codes that might be returned by LabPro_read_raw() or LabPro_send_raw().
 * You can tell that the errorcode is one of these if it is negative.
 * \ingroup misc
 */
enum LabPro_USB_Errors {
    LABPRO_ERR_USB_IO           = LIBUSB_ERROR_IO,
    LABPRO_ERR_USB_ACCESS       = LIBUSB_ERROR_ACCESS,
    LABPRO_ERR_USB_NO_DEVICE    = LIBUSB_ERROR_NO_DEVICE,
    LABPRO_ERR_USB_PIPE         = LIBUSB_ERROR_PIPE,
    LABPRO_ERR_USB_INTERRUPTED  = LIBUSB_ERROR_INTERRUPTED,
    LABPRO_ERR_USB_NO_MEM       = LIBUSB_ERROR_NO_MEM,
    LABPRO_ERR_USB_OTHER        = LIBUSB_ERROR_OTHER
};

/** \brief Thin wrapper around libusb_context
 * \ingroup init_deinit
 */
typedef struct {
    libusb_context *usb_link;
} LabPro_Context;

/** \brief Struct representing a LabPro device
 * \ingroup labpro_interface
 */
typedef struct {
    libusb_device_handle *device_handle;
    
    /** \brief Whether the underlying USB device handle is open. */
    bool is_open;
    
    /** \brief Whether there is a pending transfer request.
     * Some commands do not return data so it's OK to send them
     * even if another thread is collecting data. Others require
     * waiting.
     */
    bool is_busy;
    
    /** \brief In FastMode (20µs between samples), we cannot send any commands
     * or we will interrupt the sampling. This shouldn't cause any problems
     * because FastMode can only run for a fraction of a second before LabPro's
     * RAM is filled up.
     */
    bool is_fastmode_running;
    
    /** \brief Whether data is currently being collected.
     * This doesn't determine whether we should be sending commands, but allows
     * sanity checks on e.g. LabPro_reset().
     */
    bool is_collecting_data;
    
    /** \brief The USB "in" endpoint address. */
    unsigned char in_endpt_addr;
    
    /** \brief The USB "out" endpoint address. */
    unsigned char out_endpt_addr;
    
    /** \brief How long libusb waits before timing out on a transfer. Default is 5000. */
    unsigned int timeout;
    
    /** \brief Current firmware version 
     * From the LabPro Technical Reference Manual, the format is X.MMmms
     * (Product Code.Major.Minor.Step)
     */
    char software_id[7];
    int errorcode;
    enum LabPro_Battery_Level battery_level;
    
} LabPro;

/** \brief Struct acting as an array of LabPros
 * \ingroup init_deinit
 */
typedef struct {
    /** \brief Number of available LabPros. Max is 5. */
    unsigned short num;
    
    /** \brief Array of LabPro. */
    LabPro *labpros[5];
} LabPro_List;

/** \brief Struct representing a "data session"
 * Data sessions are an abstraction over the LabPro's command-oriented data collection
 * system. You create a data session for each channel (running LabPro_check_data_session()
 * every time the user modifies a value), then you "stage" the data sessions with
 * LabPro_stage_data_session(), then run a final check for conflicts with
 * LabPro_check_sessions_for_conflicts() before submitting the sessions to the LabPro with
 * LabPro_start_data_collection().
 * 
 * Of course, you may wish to have some sort of logic in your application as to when to
 * start collecting from a previously-disabled channel, change sampling rate, etc. LabPro
 * provides some options for triggering, but if those are insufficient, you can simply
 * stop data collection early with LabPro_stop_data_collection(), make any necessary changes
 * to the LabPro_Data_Session structs, re-stage them, re-check them, and start data collection
 * again.
 * \ingroup data_collection
 */
typedef struct {
    /** \brief The channel to use */
    enum LabPro_Channels channel;
    
    /** \brief The type of data to collect (analog channels).
     * For most analog sensors you should use LABPRO_CHANOP_AUTOID.
     * If channel is not an analog channel, set this to zero.
     */
    enum LabPro_Analog_Chan_Operations analog_op;
    
    /** \brief The type of data to collect (sonic channels).
     * I do NOT recommend using anything but LABPRO_DISTANCE_AND_DT_METERS
     * because making the LabPro calculate velocity and acceleration just
     * causes the circuitry to get hot and return inaccurate or jittery
     * data.
     * 
     * If channel is not a sonic channel, set this to zero.
     * 
     * If the sampling mode is non-realtime, you cannot get velocity and
     * acceleration from the LabPro. This is the opposite of post-processing
     * for analog channels (see below).
     */
    enum LabPro_Sonic_Chan_Operations sonic_op;
    
    /** \brief Post processing
     * LabPro can perform the first and second derivatives with respect
     * to time for you. However, I do not recommend using this, because
     * it will mean more work for the poor little LabPro, and when the
     * LabPro gets warm, the measurements tend to get inaccurate.
     * 
     * This should be LABPRO_POSTPROC_NONE if in realtime sampling mode
     * (Because only one point is stored at a time in realtime mode) or
     * when the channel is not an analog channel.
     */
    enum LabPro_Analog_PostProc postproc;
    
    /** \brief Sampling mode: realtime or non-realtime.
     * The names are a bit misleading, because LabPro itself will
     * always capture in real-time. However, you can think of this
     * setting as the way the _host_ needs to sample the LabPro.
     * In real-time mode, the host has to request data from the LabPro
     * in real-time because the LabPro does not store any data onboard.
     * In non-real-time mode, the LabPro will record data as it collects
     * it to ensure that no data points are lost.
     * 
     * If you will just be monitoring, not recording data, use
     * non-real-time mode or command 9. (Though if you are monitoring for a condition
     * to start recording, consider setting a trigger.)
     * 
     * If you want to record data but still show the data collection
     * progress, you can use non-real-time mode, but call LabPro_data_sample()
     * to get the last data point.
     */
    enum LabPro_Sampling_Modes sampling_mode;
    
    bool use_conversion_eqn;
    char* onboard_conversion_equation;
    bool use_sonic_temp_compensation;
    char* sonic_temp_compensation_equation;
} LabPro_Data_Session;

/** \brief Initialize liblabpro. Currently just a wrapper around libusb_init().
 * 
 * \param context A pointer to a liblabpro context to be populated.
 * \return One of the \ref LabPro_USB_Errors error codes or zero on success. If there is an
 *         error, it will be translated with libusb_strerror() and printed to the console.
 * 
 * \ingroup init_deinit
 */
int LabPro_init(LabPro_Context* context);

/** \brief De-initialize liblabpro.
 * \ingroup init_deinit
 */
void LabPro_exit(LabPro_Context* context);

/** \brief Obtain a list connected LabPro devices.
 * 
 * This function attempts to open and claim the USB interface for each LabPro,
 * so the application should call LabPro_close_labpro() on each LabPro in the
 * LabPro_List that will not be used.
 * 
 * \param context A pointer to the liblabpro context.
 * \return A struct containing a pointer to an array of up to
 *         5 LabPro devices and the number of found devices.
 * 
 * \ingroup init_deinit
 */
LabPro_List LabPro_list_labpros(LabPro_Context* context);

/** \brief Close the LabPro
 * 
 * Close a LabPro, releasing its USB interface and closing its libusb device handle.
 * 
 * \param labpro A pointer to the LabPro to close.
 * 
 * \ingroup init_deinit
 */
void LabPro_close_labpro(LabPro* labpro);

/** \brief Send a reset command to the specified LabPro.
 * 
 * This clears all stored data collection info, error info,
 * and channel setup. This does not erase the LabPro's flash
 * memory, only the RAM.
 * 
 * \param labpro Pointer to the LabPro to act on.
 * \param force Whether to force a reset even if the LabPro is busy
 * \return int One of the \ref LabPro_Errors codes or LABPRO_OK
 * 
 * \ingroup labpro_interface
 */
int LabPro_reset(LabPro* labpro, bool force);

/** \brief Check a data session for problems before running it on the LabPro
 * 
 * GUI programs should call this every time a setting is changed so that
 * the user can be warned.
 * 
 * \param session Pointer to data session to check
 * \param errors Pointer to pointer to an array of errors that the session may have
 * \return The number of errors encountered.
 * 
 * \ingroup data_collection
 */
int LabPro_check_data_session(LabPro_Data_Session* session, int** errors);

/** \brief Send a raw command to the LabPro.
 * 
 * This is for internal or console purposes; you shouldn't need to use it.
 * It sends the string specified by command directly to the LabPro,
 * only modifying it by adding a trailing carriage return (CR) character.
 * It does not check whether the LabPro is busy. It should not be used
 * for anything but text data, because it uses string functions to get
 * the length of the command string.
 * 
 * The memory pointed to by `command` is not freed by this function.
 * 
 * \param labpro The LabPro to write to
 * \param command Full command string to send to the LabPro
 * \param length_transferred The actual bytes transferred (in case an error occurs)
 * \return One of the \ref LabPro_Errors errocodes, LABPRO_OK, or, if the return value is negative,
 *         it is one of the \ref LabPro_USB_Errors errorcodes.
 * 
 * \ingroup internal
 */
int LabPro_send_raw(LabPro* labpro, char* command, int* length_transferred);

/** \brief Read raw bytes from the LabPro.
 * 
 * This is for internal or console purposes; you shouldn't need to use it.
 * It reads as many bytes as possible from the LabPro, then returns the
 * string. If there is trailing garbage due to the LabPro's response not
 * being a multiple of 64 bytes (the LabPro's USB packet size), it is NOT
 * stripped (see LabPro_trim_response() for that purpose). If this function
 * encounters six errors from libusb, it will assume that there is something
 * wrong and abort with an error code from LabPro_USB_Errors.
 * 
 * Even if this function returns LABPRO_ERR_NO_MEM, some of the data
 * may have been transferred, because this function calls realloc() after
 * each 64-byte packet.
 * 
 * \param labpro The LabPro to read from
 * \param string Pointer to char array that will hold the data
 * \param length The number of bytes that were read. This will always be a multiple of 64.
 * \return One of the \ref LabPro_Errors error codes, LABPRO_OK, or, if the
 *         return value is negative, it is one of the \ref LabPro_USB_Errors errorcodes.
 * 
 * \ingroup internal
 */
int LabPro_read_raw(LabPro* labpro, char** string, int* length);

/** \brief Trim trailing junk that the LabPro sent
 * Since the LabPro always returns data in multiples of 64 bytes, the
 * last packet is likely to contain junk following the actual
 * data. This replaces the trailing carriage return character and
 * everything that follows it with NULL characters.
 * 
 * \param string The string to trim
 * \param length The length of the string as set by LabPro_read_raw
 * 
 * \ingroup internal
 */
void LabPro_trim_response(char* string, int length);

void LabPro_handle_device_disconnect(LabPro* labpro);
