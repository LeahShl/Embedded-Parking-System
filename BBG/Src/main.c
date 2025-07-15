/**
 * @file main.c
 * @author Leah
 * @brief Parking system project file for BBG
 * @date 2025-07-14
 */
#include "config.h"
#include "i2c_process.h"
#include "eth_process.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    Config cfg;
    if (load_config(&cfg) != 0)
    {
        fprintf(stderr, "Failed to load config\n");
        return EXIT_FAILURE;
    }

    printf("Loaded config:\n");
    printf("  i2c_bus    = %s\n", cfg.i2c_bus);
    printf("  i2c_addr   = 0x%02X\n", cfg.i2c_addr);
    printf("  server_ip  = %s\n", cfg.server_ip);
    printf("  server_port= %d\n", cfg.server_port);

    int pipefd[2];
    if (pipe(pipefd) < 0)
    {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return 1;
    }
    else if (pid == 0)
    {
        // Child will run I2C process
        close(pipefd[0]);
        run_i2c_process(pipefd[1], &cfg);

        exit(EXIT_FAILURE); // Ensure child dies on failure
    }
    else
    {
        // Parent will run ETH process
        close(pipefd[1]); 
        run_eth_process(pipefd[0], &cfg);

        wait(NULL); // Parent should live
    }

    // Shouldn't reach here
    return EXIT_SUCCESS;
}
