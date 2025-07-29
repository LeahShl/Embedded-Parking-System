/*
 * i2c_send.c
 *
 *  Created on: Jul 13, 2025
 *      Author: leah
 */
#include "gps_sim.h"
#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <time.h>

#define I2C_TIMO 1000            // I2C timeout ms
#define TS_BUF_SIZE 64           // Timastamp buffer size

extern I2C_HandleTypeDef hi2c1;
extern osMessageQueueId_t gpsMsgQueueHandle;

static void utc_to_str(uint32_t utc_sec, char *buf);

void StartI2CSenderTask(void *argument)
{
	gps_msg_t msg;
	while (1)
	{
		if (osMessageQueueGet(gpsMsgQueueHandle, &msg, 0, osWaitForever) == osOK)
		{
			char *type_str;
			if(msg.msg_type == MSG_IDLE) type_str = "IDLE";
			else if(msg.msg_type == MSG_START) type_str = "START";
			else if(msg.msg_type == MSG_STOP) type_str = "STOP";

			char buf[TS_BUF_SIZE];
			utc_to_str(msg.utc_seconds, buf);

			printf("Sending %s message: license=%08lu, %s, lat=%.6f, lon=%.6f\n",
		           type_str, (unsigned long)msg.license_id, buf,
		           msg.latitude, msg.longitude);

			HAL_StatusTypeDef ret = HAL_I2C_Slave_Transmit(&hi2c1, (uint8_t*)&msg, sizeof(msg), I2C_TIMO);

			if (ret != HAL_OK)
			{
				printf("I2C Transmit failed with code: %d\n", ret);
			}
		}

	}
}

static void utc_to_str(uint32_t utc_sec, char *buf)
{
	time_t utc_tim = utc_sec;
	struct tm *ts;

	ts = gmtime(&utc_tim);

	strftime(buf, TS_BUF_SIZE, "%Y-%m-%d %H:%M:%S UTC", ts);
}
