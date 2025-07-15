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
#include <signal.h>

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
    printf("  server_port= %d\n\n", cfg.server_port);

    int pipefd[2];
    if (pipe(pipefd) < 0)
    {
        perror("pipe");
        return 1;
    }

    pid_t i2c_pid = fork();
    if (i2c_pid == 0)
    {
        close(pipefd[0]);
        run_i2c_process(pipefd[1], &cfg);

        exit(EXIT_FAILURE); // Shouldn't reach here
    }

    pid_t eth_pid = fork();
    if (eth_pid == 0)
    {
        /* 
          Temporary closed socket (for example, server briefly disconnects)
          will cause a SIGPIPE, which normally kills the eth proccess. Ignoring this
          signal will keep the system running even when the server is unavailable.
        */
        signal(SIGPIPE, SIG_IGN);

        close(pipefd[1]); 
        run_eth_process(pipefd[0], &cfg);

        exit(EXIT_FAILURE); // Shouldn't reach here
    }


    // Shouldn't reach here
    int status;
    waitpid(i2c_pid, &status, 0);
    waitpid(eth_pid, &status, 0);
    return EXIT_SUCCESS;
}
