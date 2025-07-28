/*
 * gps_sim.h
 *
 *  Created on: Jul 13, 2025
 *      Author: leah
 */

#ifndef INC_GPS_SIM_H_
#define INC_GPS_SIM_H_

#include <stdint.h>

#define START_LAT 32.0263f       // Starting latitute
#define START_LON 34.8257f       // Starting longitude

#define LAT_MIN 29.5f            // Minimum latitute
#define LAT_MAX 33.3f            // Maximun latitude
#define LON_MIN 34.2f            // Minimum longitude
#define LON_MAX 35.9f            // Maximum longitude

#define MOVE_STEP_MIN -0.0005f   // Minimum movement per step
#define MOVE_STEP_MAX  0.0005f   // Maximum movement per step

#define PARK_INTERVAL_MIN 5      // Minimum interval between parkings (sec)
#define PARK_INTERVAL_MAX 15     // Maximum interval between parkings (sec)
#define PARK_DURATION_MIN 1      // Minimum parking duration (sec)
#define PARK_DURATION_MAX 10     // Maximum parking duration (sec)

#define LICENSE_ID 12345678U     // Device's license id

typedef enum {
    MSG_IDLE = 0,
    MSG_START = 1,
    MSG_STOP = 2
} msg_type_t;

typedef struct __attribute__((packed)) {
    uint8_t msg_type;
    uint32_t license_id;
    uint32_t utc_seconds;
    float latitude;
    float longitude;
} gps_msg_t;

#endif /* INC_GPS_SIM_H_ */
