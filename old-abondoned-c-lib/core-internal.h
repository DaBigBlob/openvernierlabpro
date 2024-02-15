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
 */

#pragma once
#include <libusb-1.0/libusb.h>
#include <stdbool.h>
#include "backends/labpro/labpro-internal.h"

/** \brief The main state manager "object"
 * 
 * Stores info about callbacks, debug options, etc.
 * Also stores the libusb context.
 */
typedef struct {
    bool initialized;
    enum LabPro_Error_Severity max_console_log_severity;
    libusb_context* usb_link;
} LabPro_State_Manager;
