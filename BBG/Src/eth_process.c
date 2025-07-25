/**
 * @file eth_process.c
 * @author Leah
 * @brief Multiprocessing networking over ethernet for parksys
 * @date 2025-07-15
 * 
 */
#include "eth_process.h"
#include "gps_msg.h"
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <syslog.h>

/**
 * @brief Deals with connection to the server
 * 
 * @param ip Server's IP
 * @param port TCP port
 * @return int sockfd if successful, -1 otherwise
 */
static int connect_to_server(const char *ip, int port);

void run_eth_process(int read_fd, const Config *cfg)
{
    int sockfd = -1;

    while (1)
    {
        // Read from IPC pipe
        gps_msg_t msg;
        ssize_t rd = read(read_fd, &msg, sizeof(msg));
        if (rd < 0)
        {
            syslog(LOG_ERR, "[ETH] Read error: %s\n", strerror(errno));
            usleep(100000);
            continue;
        }

        if (rd != sizeof(msg))
        {
            syslog(LOG_WARNING, "[ETH] Short read: %zd bytes, expected %zu\n", rd, sizeof(msg));
            continue;
        }

        syslog(LOG_INFO, "[ETH] msg_type=%s(%u), license=%08u, utc_sec=%u, lat=%.6f, lon=%.6f\n",
               type_str(msg.msg_type),
               msg.msg_type,
               msg.license_id,
               msg.utc_sec,
               msg.latitude,
               msg.longitude);

        // Send message to server with infinite retries,
        // reconnect if needed
        while (1)
        {
            if (sockfd < 0)
            {
                sockfd = connect_to_server(cfg->server_ip, cfg->server_port);
                if (sockfd < 0)
                {
                    syslog(LOG_WARNING, "[ETH] Retry in 2 seconds...\n");
                    sleep(2);
                }
            }

            ssize_t wr = write(sockfd, &msg, sizeof(msg));
            if (wr == sizeof(msg))
                break; // success

            syslog(LOG_ERR, "[ETH] Write failed: %s. Reconnecting...\n", strerror(errno));
            close(sockfd);
            sockfd = -1;
            sleep(1);

        }
    }

    if (sockfd >= 0) close(sockfd); // Shouldn't reach here
}

static int connect_to_server(const char *ip, int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        syslog(LOG_ERR, "[ETH] socket: %s", strerror(errno));
        return -1;
    }

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv.sin_addr) <= 0)
    {
        syslog(LOG_ERR, "[ETH] Invalid IP: %s\n", ip);
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr*)&serv, sizeof(serv)) < 0)
    {
        syslog(LOG_ERR, "[ETH] connect: %s", strerror(errno));
        close(sockfd);
        return -1;
    }

    syslog(LOG_INFO, "[ETH] Connected to %s:%d\n", ip, port);
    return sockfd;
}
