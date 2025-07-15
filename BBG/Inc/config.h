/**
 * @file config.h
 * @author Leah
 * @brief Config file I/O handling
 * @date 2025-07-15
 * 
 */
#pragma once

#define DEFAULT_I2C_BUS "/dev/i2c-1"
#define DEFAULT_I2C_ADDR 0x10
#define DEFAULT_SERVER_IP "192.168.1.71"
#define DEFAULT_SERVER_PORT 12321
#define DEFAULT_LOG_PATH "/var/log/parksys/parksys.log"
#define DEFAULT_SERVICE_NAME "parksys.service"
#define DEFAULT_SERVICE_PATH "/etc/systemd/system/parksys.service"
#define CONFIG_PATH "/etc/parksys/parksys.config"

typedef struct {
    char i2c_bus[32];        // path to i2c device, e.g. "/dev/i2c-1"
    int  i2c_addr;           //static void install_service(const Config *cfg) i2c slave address, e.g. 0x10
    char server_ip[64];      // Server IP address, e.g. "192.168.1.100"
    int  server_port;        // Server port, e.g. 12345
    char log_path[512];      // Path to log file
    char service_name[64];  // Systemd service name
    char service_path[512];  // Path to systemd service file
} Config;

/**
 * @brief Loads or creates the config file and fills the Config struct.
 *        If the file does not exist, it is created with default values.
 *
 * @param cfg Pointer to Config struct to populate
 * @return 0 on success, -1 on failure
 */
int load_config(Config *cfg);
