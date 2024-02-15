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

#include <liblabpro.h>
#include <stdio.h>
#include <editline/readline.h> // This has to after stdio
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

int split_cmd_args(char* command, char*** argv)
{
    char* current_command;
    if (command[0] == '!' && command[1] != ' ')
        current_command = command + 1;
    else {
        printf(":: Sanity check failed; the command does not begin with an exclamation mark or the \"!\" is followed by a space!\n");
        return(0);
    }
    
    *argv = malloc(1 * sizeof(char*));
    if (*argv == NULL) {
        printf(":: Failed to allocate memory!\n");
        return(0);
    }
    int i = 0;
    while(true) {
        char* first_space = strstr(current_command, " ");
        if (first_space != NULL) {
            int first_space_offset = (uintptr_t)first_space - (uintptr_t)current_command;
            (*argv)[i] = calloc(first_space_offset + 2, sizeof(char)); // Because first_space_offset is the zero-indexed location, and we also need space for a null byte at the end.
            memcpy((*argv)[i], current_command, first_space_offset + 1);
        }
        else {
            (*argv)[i] = calloc(strlen(current_command) + 1, sizeof(char));
            memcpy((*argv)[i], current_command, strlen(current_command));
            return(i + 1);
        }
        char** argv_new = realloc(*argv, (i * sizeof(char*)) + (2 * sizeof(char*)));
        if (argv_new == NULL) {
            printf(":: Failed to allocate memory!\n");
            return(i + 1);
        }
        else
            *argv = argv_new;
        
        ++i;
        current_command = first_space + 1;
    }
}

void free_cmd_argv(int argc, char** argv) {
    for (int i = 0; i < argc; ++i)
        free(argv[i]);
}

void print_help() {
    printf(":: Help for the LabPro USB Console:\n");
    printf("::   Any input starting with an exclamation point will be interpreted as an internal command.\n");
    printf("::   The available commands are:\n");
    printf("::     !quit: Exit the console.\n");
    printf("::     !help: Show this information.\n");
    printf("::   Any input not starting with an exclamation point will be sent to the first connected\n");
    printf("::   LabPro device found. A carriage-return (CR) character is appended to the input, but\n");
    printf("::   no error checking is performed, so be careful!\n");
}

int main(int argc, char** argv) {
    printf("LabPro USB Console (http://liblabpro.sf.net)\n");
    printf("--------------------------------------------\n");
    
    int fake_shell = false;
    LabPro* selected_labpro;
    if (argc > 1)
    {
        if (strcmp(argv[1], "--fake") == 0)
            fake_shell = true;
        else if (strcmp(argv[1], "--help") == 0) {
            printf("For help, start the shell and enter \"!help\" (without quotes) and hit enter.\n");
            printf("Run labpro-console with the \"--fake\" flag to enter a fake shell without a LabPro connected.\n");
            return 0;
        }
        else {
            printf("Invalid flag. Run labpro-console with the \"--help\" flag for help.\n");
            return 1;
        }
    }
    
    if (!fake_shell) {
        printf(":: Initializing liblabpro...\n");
        LabPro_Context ctx;
        LabPro_init(&ctx);
        
        printf(":: Searching for connected LabPro devices...\n");
        LabPro_List list = LabPro_list_labpros(&ctx);
        
        if (list.num == 0) {
            printf(":: No LabPro devices found; aborting.\n");
            LabPro_exit(&ctx);
            return(0);
        }
        printf(":: Found %d LabPro devices.\n", list.num);
        printf(":: Auto-selecting the first discovered LabPro device.\n");
        selected_labpro = list.labpros[0];
        if (list.num > 1) {
            for (int i = 1; i < list.num; ++i) {
                LabPro_close_labpro(list.labpros[i]);
                free(list.labpros[i]);
            }
        }
    }
    else
        printf(":: Starting a fake shell.\n");
    
    printf(":: Welcome to the LabPro USB Console.\n");
    printf(":: Lines starting with \"::\" or \"[liblabpro]\" come from this console itself, or liblabpro.\n");
    printf(":: Lines starting with \"->\" have been sent by the LabPro.\n");
    printf(":: When you see a blank line starting with \"<-\", it means that the text you type will be sent to the LabPro,\n");
    printf(":: unless it starts with an exclamation mark (!), in which case it is interpreted as a console command and is not sent to the LabPro.\n");
    printf(":: Type !help for a list of commands.\n");
    
    while (true) {
        char* message = readline("<- ");
        if (message[0] == '!') {
            char** argv_cmd;
            int argc_cmd = split_cmd_args(message, &argv_cmd);
            if (argc_cmd > 0) {
                if (strcmp(argv_cmd[0], "quit") == 0)
                    exit(0);
                else if (strcmp(argv_cmd[0], "help") == 0)
                    print_help();
                else
                    printf("No command found by the name \"%s\". Try \"!help\".\n", argv_cmd[0]);
                
                free_cmd_argv(argc_cmd, argv_cmd);
            }
        }
        else {
            if (!fake_shell) {
                
                
            }
            else {
                printf("-> Fake response from LabPro.\n");
            }
        }
    }
    return 0;
}
