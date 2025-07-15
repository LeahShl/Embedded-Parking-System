/**
 * @file eth_process.h
 * @author Leah
 * @brief Multiprocessing networking over ethernet for parksys
 * @date 2025-07-15
 * 
 */
#pragma once

#include "config.h"

/**
 * @brief Runs the ETH process loop.
 *
 * @param read_fd Read-end of the IPC pipe
 * @param cfg Pointer to config 
 */
void run_eth_process(int read_fd, const Config *cfg);