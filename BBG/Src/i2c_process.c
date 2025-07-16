/**
 * @file i2c_process.c
 * @author Leah
 * @brief Multiprocessing I2C communication for parksys
 * @date 2025-07-15
 * 
 */
#include "i2c_process.h"
#include "gps_msg.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <syslog.h>

void run_i2c_process(int write_fd, const Config *cfg)
{
    int fd = open(cfg->i2c_bus, O_RDWR);
    if (fd < 0)
    {
        syslog(LOG_ERR, "[I2C] open i2c_bus: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (ioctl(fd, I2C_SLAVE, cfg->i2c_addr) < 0)
    {
        syslog(LOG_ERR, "[I2C] ioctl I2C_SLAVE: %s", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "[I2C] Listening on %s @0x%02X ...\n", cfg->i2c_bus, cfg->i2c_addr);

    while (1)
    {
        uint8_t buf[MSG_LEN];
        ssize_t rd = read(fd, buf, MSG_LEN);

        if (rd < 0)
        {
            if (errno == EAGAIN || errno == EINTR || errno == EBUSY || errno == EIO || errno == ENXIO)
            {
                syslog(LOG_WARNING, "[I2C] Warning: read failed (%s), retrying...\n", strerror(errno));
                usleep(500000);
                continue;
            }
            continue;
        }

        if (rd != MSG_LEN)
        {
            syslog(LOG_WARNING, "[I2C] short read: %zd/%d bytes, retrying...\n", rd, MSG_LEN);
            usleep(100000);
            continue;
        }

        gps_msg_t msg;
        memcpy(&msg, buf, sizeof(msg));

        if (msg.msg_type > 2)
        {
            syslog(LOG_WARNING, "[I2C] Invalid msg_type %u, skipping\n", msg.msg_type);
            continue;
        }

        if (msg.msg_type == 0)
        syslog(LOG_INFO, "[I2C] msg_type=%s(%u), license=%08u, utc_sec=%u, lat=%.6f, lon=%.6f\n",
               type_str(msg.msg_type),
               msg.msg_type,
               msg.license_id,
               msg.utc_sec,
               msg.latitude,
               msg.longitude);

        // Forward only START/STOP messages to pipe
        if (msg.msg_type == 1 || msg.msg_type == 2) {
            ssize_t wr = write(write_fd, &msg, sizeof(msg));
            if (wr != sizeof(msg)) {
                syslog(LOG_ERR, "[I2C] Failed to write to pipe: %s\n", strerror(errno));
            }
        }

        sleep(1);
    }

    close(fd);
}