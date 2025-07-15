/**
 * @file config.h
 * @author Leah
 * @brief Config file I/O handling
 * @date 2025-07-15
 * 
 */
#pragma once

#define CONFIG_DIR     "Parksys"                 // Config directory, relative to home directory
#define CONFIG_PATH    "Parksys/parksys.config"  // Config file path, relative to home directory

typedef struct {
    char i2c_bus[32];        // path to i2c device, e.g. "/dev/i2c-1"
    int  i2c_addr;           // i2c slave address, e.g. 0x10
    char server_ip[64];      // Server IP address, e.g. "192.168.1.100"
    int  server_port;        // Server port, e.g. 12345
} Config;

/**
 * @brief Loads or creates the config file and fills the Config struct.
 *        If the file does not exist, it is created with default values.
 *
 * @param cfg Pointer to Config struct to populate
 * @return 0 on success, -1 on failure
 */
int load_config(Config *cfg);
