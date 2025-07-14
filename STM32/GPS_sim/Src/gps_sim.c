/*
 * gps_sim.c
 *
 *  Created on: Jul 13, 2025
 *      Author: leah
 */
#include "main.h"
#include "gps_sim.h"
#include "cmsis_os.h"
#include <stdlib.h>
#include <string.h>

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

// Private utility functions
static uint32_t rand_uint_range(uint32_t min, uint32_t max);
static float rand_float_range(float min, float max);
static void update_position_randomly(void);
static void send_gps_msg(msg_type_t type);

// Globals
volatile uint32_t utc_seconds = 0;
static float lat = 0.0f;
static float lon = 0.0f;
static uint8_t is_parking = 0;
static uint32_t park_counter = 0;
static uint32_t park_duration = 0;
static uint32_t until_next_park = 0;

extern osMessageQueueId_t gpsMsgQueueHandle;

void StartGPSTask(void *argument)
{
	srand(0);
	lat = rand_float_range(LAT_MIN, LAT_MAX);
	lon = rand_float_range(LON_MIN, LON_MAX);

	until_next_park = rand_uint_range(PARK_INTERVAL_MIN, PARK_INTERVAL_MAX);

	while (1)
	{
		if (is_parking)
		{
			if (park_counter == 0)
			{
				send_gps_msg(MSG_START);
			}
			else if (park_counter >= park_duration)
			{
				send_gps_msg(MSG_STOP);
				is_parking = 0;
				until_next_park = rand_uint_range(PARK_INTERVAL_MIN, PARK_INTERVAL_MAX);
				park_counter = 0;
			}
			else
			{
				send_gps_msg(MSG_IDLE);
			}
			park_counter++;
		}
		else
		{
			update_position_randomly();
			send_gps_msg(MSG_IDLE);
			if (--until_next_park == 0)
			{
				is_parking = 1;
				park_duration = rand_uint_range(PARK_DURATION_MIN, PARK_DURATION_MAX);
				park_counter = 0;
			}
		}

		osDelay(1000);
	}
}

static float rand_float_range(float min, float max)
{
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

static uint32_t rand_uint_range(uint32_t min, uint32_t max)
{
    return min + (rand() % (max - min + 1));
}

static void update_position_randomly(void)
{
    float d_lat = rand_float_range(MOVE_STEP_MIN, MOVE_STEP_MAX);
    float d_lon = rand_float_range(MOVE_STEP_MIN, MOVE_STEP_MAX);

    lat += d_lat;
    lon += d_lon;

    if (lat < LAT_MIN) lat = LAT_MIN;
    if (lat > LAT_MAX) lat = LAT_MAX;
    if (lon < LON_MIN) lon = LON_MIN;
    if (lon > LON_MAX) lon = LON_MAX;
}

void send_gps_msg(msg_type_t type)
{
	gps_msg_t msg;
	msg.msg_type = type;
	msg.license_id = LICENSE_ID;
	msg.utc_seconds = utc_seconds;
	msg.latitude = lat;
	msg.longitude = lon;

	osMessageQueuePut(gpsMsgQueueHandle, &msg, 0, osWaitForever);
}
