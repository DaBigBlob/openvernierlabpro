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
 * \defgroup Core Core library structures
 * \defgroup Sensors Data structures describing sensors etc.
 * \defgroup Init_Deinit Initialization/Deinitialization of liblabpro
 * \defgroup Data_Collection Data Collection
 * \defgroup Misc Miscellaneous
 * \defgroup Internal Internal functions, enums, etc.
 */

/** \mainpage liblabpro API Reference
 * 
 * Introduction
 * ------------
 * liblabpro is an open-source library that allows you to collect data from Vernier LabPro sensor interfaces.
 * Special thanks to Vernier for publishing the [LabPro Technical Reference Manual](http://www2.vernier.com/labpro/labpro_tech_manual.pdf)
 * and Ben Crowell for his additional reverse engineering on [FreeLab](http://lightandmatter.com/freelab/).
 * 
 * Overview
 * --------
 * See the Modules page for more details, but here are the basic steps:
 * 
 * - Initialize with LabPro_libinit()
 * - Search for LabPro interfaces with LabPro_list_labpros()
 * - Choose the LabPro you want to use and run LabPro_close_labpro() on all the others.
 * - Reset the device with LabPro_reset()
 * - Fill a LabPro_Data_session struct with parameters, then verify that it will work
 *   with LabPro_check_data_session().
 * - To be continued...
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>

/** \brief Types of errors
 * Because liblabpro is designed to incorporate support for more than just LabPro devices,
 * it is necessary to provide a way of describing where an error came from.
 * 
 * \ingroup Core
 */
enum LabPro_Error_Types {
    /** \brief An error not related to actual data acquisition, but rather some internal problem such
     * failure to allocate memory, problems with gettext, or the like.
     */
    LABPRO_ERRORTYPE_GENERIC,
    /** \brief An error related to sensors but not a specific interface
     * If there is a problem with a sensor but not with the interface itself, it goes here.
     * Example: trying to burn a calibration page onto a non-"smart" sensor.
     */
    LABPRO_ERRORTYPE_SENSOR,
    /** \brief An error related to use of the LabPro interface itself. */
    LABPRO_ERRORTYPE_BACKEND_LABPRO,
    
    /** \brief An error related to Go! devices.
     * Currently there is no support for Go! devices in liblabpro. However, the library is
     * designed to accomodate this in the future.
     */
    LABPRO_ERRORTYPE_BACKEND_GO_USB
};

/** \brief Error severity levels.
 * 
 * \ingroup Core
 */
enum LabPro_Error_Severity {
    /** \brief The error prevents further execution. (e.g. out-of-memory) */
    LABPRO_ERRORSEVERITY_FATAL,
    /** \brief The error requires immediate intervention (not sure if I'll use this) */
    LABPRO_ERRORSEVERITY_ALERT,
    LABPRO_ERRORSEVERITY_CRITICAL,
    LABPRO_ERRORSEVERITY_ERROR,
    LABPRO_ERRORSEVERITY_WARNING,
    LABPRO_ERRORSEVERITY_NOTICE,
    LABPRO_ERRORSEVERITY_INFO,
    LABPRO_ERRORSEVERITY_DEBUG
};

/** \brief Struct representing an error
 * Most liblabpro functions will return this type.
 * 
 * \ingroup Core
 */
typedef struct {
    /** \brief The error type. See LabPro_Error_Types */
    enum LabPro_Error_Types type;
    
    /** \brief The error code. Different error types have different sets of error codes.
     * - For errors of type LABPRO_ERRORTYPE_GENERIC, see LabPro_Errorcodes_Generic.
     * - For errors of type LABPRO_ERRORTYPE_SENSOR, see LabPro_Errorcodes_Sensor.
     * - For errors of type LABPRO_ERRORTYPE_BACKEND_LABPRO, see LabPro_Errorcodes_Backend_LabPro
     * - For errors of type LABPRO_ERRORTYPE_BACKEND_GO_USB (not used yet), see LabPro_Errorcodes_Backend_Go_USB
     */
    int code;
    
    /** \brief An extra code, such as the error code from a function call outside of this library.
     * For example, if the LabPro backend encounters an error communicating with a LabPro device, this will store
     * the error code returned by libusb.
     */
    int extra_code;
    
    /** \brief The severity of the error. */
    enum LabPro_Error_Severity severity;
    
    /** \brief Translated error message string. */
    char* message;
    
    /** \brief Translated error message string corresponding to extra_code
     * In case there is an error returned by e.g. libusb, this will contain the string that the library provides.
     */
    char* extra_message;
} LabPro_Error;

/** \brief Error codes for the core library
 * 
 * \ingroup Core
 */
enum LabPro_Errorcodes_Generic {
    LABPRO_ERR_GENERIC_OK,
    LABPRO_ERR_GENERIC_NO_MEM,
    LABPRO_ERR_GENERIC_GETTEXT
};

/** \brief This should be your first call into liblabpro.
 * It initializes the global liblabpro state manager singleton with a
 * libusb context and gets everything ready to start working.
 * 
 * \ingroup Core
 */
LabPro_Error LabPro_libinit();

/** \brief Stop everything liblabpro is doing and free memory
 * 
 * \ingroup Core
 */
LabPro_Error LabPro_libexit();

/** \brief Error codes for sensors
 * 
 * \ingroup Sensors
 */
enum LabPro_Errorcodes_Sensor {
    /** \brief Everything is OK */
    LABPRO_ERR_SENSOR_OK,
    /** \brief The sensor information could not be automatically detected. */
    LABPRO_ERR_SENSOR_NO_AUTO_ID,
    /** \brief The sensor cannot store calibration tables because it is not "smart"
     * Newer analog sensors include an I2C interface for storing calibration tables.
     * If the sensor is not equipped with I2C and an attempt is made to write a
     * calibration table, this error will be returned.
     */
    LABPRO_ERR_SENSOR_NOT_SMART,
    /** \brief An attempt was made to change a sensor setting that has to be set per-interface.
     * For example, with LabPro, data collection rate for all analog sensors must be the same.
     * However, Go! sensors are kinda a sensor and an interface at the same time, so setting
     * data-collection rate per-sensor is allowed and will return this error if it is not
     * possible to do so.
     */
    LABPRO_ERR_SENSOR_INTERFACE_LEVEL_SETTING
};

/** \brief Sensor manufacturer codes
 * Currently the only one I know of is Vernier
 * 
 * \ingroup Sensors
 */
enum LabPro_Sensor_Manufacturers {
    LABPRO_SENSOR_MANUFACTURER_VERNIER = 0
};

/** \brief An (analog) sensor calibration page
 * k0, k1, and k2 are the coefficients used in the conversion equation.
 * 
 * \ingroup Sensors
 */
typedef struct {
    float k0;
    float k1;
    float k2;
    /** \brief The units stored in the sensor's calibration page */
    char units[7];
} LabPro_Sensor_Calibration_Page;

/** \brief Structure representing an analog sensor
 * 
 * Don't touch this, only use the getter/setter model to change stuff.
 * 
 * \ingroup Sensors
 */
typedef struct {
    /** \brief Whether the sensor has an I2C interface and onboard calibration storage
     * If this is false, some of the members below will not store useful information.
     */
    bool is_smart;
    /** \brief The sensor ID number, used in sensormap.xml.
     * It usually is a unique ID for a sensor model, though there are exceptions;
     * e.g. motion detector models MDO-BTD and MD-BTD both return the same ID
     */
    int id;
    /** \brief The sensor's serial number, only available with smart sensors */
    unsigned int serial_number;
    /** \brief The year component of the sensor's lot code, only available with smart sensors */
    uint8_t lotcode_year;
    /** \brief The week component of the sensor's lot code, only available with smart sensors */
    uint8_t lotcode_week;
    /** \brief The manufacturer ID of the sensor, only available with smart sensors */
    enum LabPro_Sensor_Manufacturers manufacturer;
    /** \brief The "long" sensor name
     * As stored in DDS memory (for smart sensors) or in the interface firmware (for resistor-ID sensors)
     */
    char name_long[20];
    /** \brief The "short" sensor name
     * As stored in DDS memory (for smart sensors) or in the interface firmware (for resistor-ID sensors)
     */
    char name_short[12];
    /** \brief The translated user-friendly sensor name */
    char* name_pretty;
    /** \brief Not sure what this means; you can ignore it. Only available with smart sensors. */
    uint8_t uncertainty;
    /** \brief Not sure how to parse this; ignore it. */
    uint8_t sigfigs;
    /** \brief Current requirement of sensor, in milliamps. Only available with smart sensors. */
    uint8_t current;
    /** \brief Whether to use oversampling and take an average. Only available with smart sensors. */
    uint8_t averaging;
    /** \brief Minimum time between samples, in seconds. Only available with smart sensors. */
    float min_sample_period;
    /** \brief Typical time between samples, in seconds. */
    float typical_sample_period;
    /** \brief Typical number of samples to collect */
    unsigned short typical_num_samples;
    /** \brief Time, in seconds, before readings are accurate */
    uint8_t warm_up_time;
    /** \brief LoggerPro experiment type. Not sure what this means. Only available with smart sensors. */
    uint8_t lp_experiment_type;
    /** \brief Measurement operation for LabPro Command 1 */
    uint8_t measurement_op;
    /** \brief Conversion equation type for LabPro Command 4 */
    uint8_t equation_type;
    /** \brief Suggested minimum Y-axis value on a graph */
    float y_min;
    /** \brief Suggested maximum Y-axis value on a graph */
    float y_max;
    /** \brief Suggested Y-axis tickmark increment on a graph */
    uint8_t y_scale;
    /** \brief Maximum valid calibration page index. Only available with smart sensors. */
    uint8_t max_valid_cal_idx;
    /** \brief Currently active calibration page index. Only available with smart sensors. */
    uint8_t active_cal_idx;
    /** \brief Calibration pages */
    LabPro_Sensor_Calibration_Page calibrations[3];
} LabPro_Analog_Sensor;
