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

extern I2C_HandleTypeDef hi2c1;
extern osMessageQueueId_t gpsMsgQueueHandle;

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

			printf("Sending %s message: license=%08lu, time(sec)=%lu, lat=%.6f, lon=%.6f\n",
		           type_str, (unsigned long)msg.license_id, (unsigned long)msg.utc_seconds,
		           msg.latitude, msg.longitude);

			HAL_StatusTypeDef ret = HAL_I2C_Slave_Transmit(&hi2c1, (uint8_t*)&msg, sizeof(msg), HAL_MAX_DELAY);

			if (ret != HAL_OK)
			{
				printf("I2C Transmit failed with code: %d\n", ret);
			}
		}

	}
}
