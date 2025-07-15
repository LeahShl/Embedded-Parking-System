/**
 * @file gps_msg.h
 * @author Leah
 * @brief GPS message definitions
 * @date 2025-07-15
 */

#pragma once

#include <stdint.h>

#define MSG_LEN 17  // 1 + 4 + 4 + 4 + 4

typedef struct __attribute__((packed))
{
    uint8_t  msg_type;                 // 0=idle, 1=start, 2=stop
    uint32_t license_id;               // 0-99999999
    uint32_t utc_sec;                  // seconds since reference
    float    latitude;                 // degrees
    float    longitude;                // degrees
} gps_msg_t;

/**
 * @brief Converts message type value to string
 * 
 * @param t Message type
 * @return const char* String representation of message type
 */
const char *type_str(uint8_t t);