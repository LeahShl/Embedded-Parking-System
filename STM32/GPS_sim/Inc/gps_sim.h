/*
 * gps_sim.h
 *
 *  Created on: Jul 13, 2025
 *      Author: leah
 */

#ifndef INC_GPS_SIM_H_
#define INC_GPS_SIM_H_

#include <stdint.h>

#define DEVICE_I2C_ADDR 0x10

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
