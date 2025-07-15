/**
 * @file eth_process.c
 * @author Leah
 * @brief Multiprocessing networking over ethernet for parksys
 * @date 2025-07-15
 * 
 */
#include "eth_process.h"
#include "gps_msg.h"
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void run_eth_process(int read_fd, const Config *cfg)
{
    (void)cfg;

    while (1)
    {
        gps_msg_t msg;
        ssize_t rd = read(read_fd, &msg, sizeof(msg));
        if (rd < 0) {
            fprintf(stderr, "[ETH] Read error: %s\n", strerror(errno));
            usleep(100000);
            continue;
        }

        if (rd != sizeof(msg)) {
            fprintf(stderr, "[ETH] Short read: %zd bytes, expected %zu\n", rd, sizeof(msg));
            continue;
        }

        printf("[ETH] msg_type=%s(%u), license=%08u, utc_sec=%u, lat=%.6f, lon=%.6f\n",
               type_str(msg.msg_type),
               msg.msg_type,
               msg.license_id,
               msg.utc_sec,
               msg.latitude,
               msg.longitude);
    }
}
