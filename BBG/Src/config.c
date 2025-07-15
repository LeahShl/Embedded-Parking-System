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

static const char *default_config =
    "i2c_bus=/dev/i2c-1\n"
    "i2c_addr=0x10\n"
    "server_ip=192.168.1.71\n"
    "server_port=12321\n";

int load_config(Config *cfg)
{
    const char *home = getenv("HOME");
    if (!home)
    {
        perror("HOME environment variable not set\n");
        return -1;
    }

    char dir_path[1024], config_path[512];
    snprintf(dir_path, sizeof(dir_path), "%s/%s", home, CONFIG_DIR);
    snprintf(config_path, sizeof(config_path), "%s/%s", home, CONFIG_PATH);

    printf("config dir: %s\nconfig file: %s\n", dir_path, config_path);


    // Create directory if it doesn't exist
    struct stat st = {0};
    if (stat(dir_path, &st) == -1)
    {
        if (mkdir(dir_path, 0755) < 0)
        {
            perror("mkdir");
            return -1;
        }
    }

    FILE *fd = fopen(config_path, "r");
    if (!fd)
    {
        // File doesn't exist: create with defaults
        fd = fopen(config_path, "w");
        if (!fd)
        {
            perror("fopen for writing");
            return -1;
        }

        fprintf(fd, "%s", default_config);
        fclose(fd);

        fd = fopen(config_path, "r");
        if (!fd)
        {
            perror("fopen for reading");
            return -1;
        }
    }

    // Set defaults first
    strcpy(cfg->i2c_bus, "/dev/i2c-1");
    cfg->i2c_addr = 0x10;
    strcpy(cfg->server_ip, "192.168.1.71");
    cfg->server_port = 12321;

    // Parse config file
    char line[128];
    while (fgets(line, sizeof(line), fd))
    {
        if (line[0] == '#' || line[0] == '\n')
            continue;

        char key[64], val[64];
        if (sscanf(line, "%63[^=]=%63s", key, val) == 2)
        {
            if (strcmp(key, "i2c_bus") == 0)
                strncpy(cfg->i2c_bus, val, sizeof(cfg->i2c_bus));
            else if (strcmp(key, "i2c_addr") == 0)
                cfg->i2c_addr = (int)strtol(val, NULL, 0);  // can handle hex or decimal
            else if (strcmp(key, "server_ip") == 0)
                strncpy(cfg->server_ip, val, sizeof(cfg->server_ip));
            else if (strcmp(key, "server_port") == 0)
                cfg->server_port = atoi(val);
        }
    }

    fclose(fd);
    return 0;
}
