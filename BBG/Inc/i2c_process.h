/**
 * @file i2c_process.h
 * @author Leah
 * @brief Multiprocessing I2C communication for parksys
 * @date 2025-07-15
 * 
 */
#pragma once

#include "config.h"

/**
 * @brief Runs the I2C process loop.
 *
 * @param write_fd Write-end of the IPC pipe (for sending START/STOP messages)
 * @param cfg Pointer to config containing I2C settings
 */
void run_i2c_process(int write_fd, const Config *cfg);

