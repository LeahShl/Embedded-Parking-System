/**
 * @file gps_msg.h
 * @author Leah
 * @brief GPS message definitions
 * @date 2025-07-15
 */

#pragma once

#include <stdint.h>

#define MIN_LICENSE 1000000            // Min lisence plate ID

#define LAT_MIN 29.5                   // Minimum latitute
#define LAT_MAX 33.3                   // Maximun latitude
#define LON_MIN 34.2                   // Minimum longitude
#define LON_MAX 35.9                   // Maximum longitude

#define MIN_UTC 1751371200             // Tuesday, July 1, 2025 12:00:00 PM

#define MSGT_IDLE 0                    // IDLE message type
#define MSGT_START 1                   // START message type
#define MSGT_STOP 2                    // STOP message type
#define MAX_MSGT MSGT_STOP             // Maximum message type

typedef struct __attribute__((packed))
{
    uint8_t  msg_type;                 // 0=idle, 1=start, 2=stop
    uint32_t license_id;               // 0-99999999
    uint32_t utc_sec;                  // seconds since reference
    float    latitude;                 // degrees
    float    longitude;                // degrees
} gps_msg_t;

#define MSG_LEN sizeof(gps_msg_t)

/**
 * @brief Converts message type value to string
 * 
 * @param t Message type
 * @return const char* String representation of message type
 */
const char *type_str(uint8_t t);