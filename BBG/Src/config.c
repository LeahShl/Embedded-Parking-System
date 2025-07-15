/**
 * @file config.c
 * @author Leah
 * @brief Config file I/O handling
 * @date 2025-07-15
 * 
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

int load_config(Config *cfg)
{
    FILE *fd = fopen(CONFIG_PATH, "r");
    if (!fd)
    {
        perror("fopen config");
        return -1;
    }

    // Set defaults
    strcpy(cfg->i2c_bus, DEFAULT_I2C_BUS);
    cfg->i2c_addr = DEFAULT_I2C_ADDR;
    strcpy(cfg->server_ip, DEFAULT_SERVER_IP);
    cfg->server_port = DEFAULT_SERVER_PORT;
    strcpy(cfg->log_path, DEFAULT_LOG_PATH);
    strcpy(cfg->service_name, DEFAULT_SERVICE_NAME);
    strcpy(cfg->service_path, DEFAULT_SERVICE_PATH);

    char line[256];
    while (fgets(line, sizeof(line), fd))
    {
        if (line[0] == '#' || line[0] == '\n')
            continue;

        char key[64], val[192];
        if (sscanf(line, "%63[^=]=%191s", key, val) == 2)
        {
            if (strcmp(key, "i2c_bus") == 0)
                strncpy(cfg->i2c_bus, val, sizeof(cfg->i2c_bus));
            else if (strcmp(key, "i2c_addr") == 0)
                cfg->i2c_addr = (int)strtol(val, NULL, 0);
            else if (strcmp(key, "server_ip") == 0)
                strncpy(cfg->server_ip, val, sizeof(cfg->server_ip));
            else if (strcmp(key, "server_port") == 0)
                cfg->server_port = atoi(val);
            else if (strcmp(key, "log_path") == 0)
                strncpy(cfg->log_path, val, sizeof(cfg->log_path));
            else if (strcmp(key, "service_name") == 0)
                strncpy(cfg->service_name, val, sizeof(cfg->service_name));
            else if (strcmp(key, "service_path") == 0)
                strncpy(cfg->service_path, val, sizeof(cfg->service_path));
        }
    }

    fclose(fd);
    return 0;
}