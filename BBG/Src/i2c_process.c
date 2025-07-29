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
#include <time.h>

#define SMALL_DELAY 500

/**
 * @brief memcpy wrapper that validates the data it copies
 * 
 * @param msg Destination
 * @param buf Source buffer
 * @return int 1 if successful, 0 otherwise
 */
static int memcpy_validate(gps_msg_t *msg, uint8_t *buf);

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
            if (errno == EAGAIN || errno == EINTR || errno == EBUSY
                || errno == EIO || errno == ENXIO || errno == EREMOTEIO)
            {
                syslog(LOG_ERR, "[I2C] Error: read failed (%s), retrying...\n", strerror(errno));
                usleep(SMALL_DELAY);
            }
            continue;
        }

        if (rd != MSG_LEN)
        {
            syslog(LOG_WARNING, "[I2C] short read: %zd/%d bytes, retrying...\n", rd, MSG_LEN);
            usleep(SMALL_DELAY);
            continue;
        }

        gps_msg_t msg;

        if (memcpy_validate(&msg, buf))
        {
            const time_t msg_time = msg.utc_sec;

            if (msg.msg_type == 0)
                syslog(LOG_INFO, "[I2C] msg_type=%s(%u), license=%08u, lat=%.6f, lon=%.6f, %s",
                    type_str(msg.msg_type),
                    msg.msg_type,
                    msg.license_id,
                    msg.latitude,
                    msg.longitude,
                    asctime(gmtime(&msg_time)));

            // Forward only START/STOP messages to pipe
            if (msg.msg_type == 1 || msg.msg_type == 2) {
                ssize_t wr = write(write_fd, &msg, sizeof(msg));
                if (wr != sizeof(msg)) {
                    syslog(LOG_ERR, "[I2C] Failed to write to pipe: %s\n", strerror(errno));
                }
            }
        }
    }
    close(fd);
}

static int memcpy_validate(gps_msg_t *msg, uint8_t *buf)
{
    memcpy(msg, buf, MSG_LEN);

    if (msg->msg_type > MAX_MSGT)
    {
        syslog(LOG_WARNING, "[I2C] Invalid msg_type %u, skipping\n", msg->msg_type);
        return 0;
    }

    if (msg->license_id < MIN_LICENSE)
    {
        syslog(LOG_WARNING, "[I2C] Invalid license_id %u, skipping\n", msg->license_id);
        return 0;
    }

    if (msg->latitude < LAT_MIN || msg->latitude > LAT_MAX
        || msg->longitude < LON_MIN || msg->longitude > LON_MAX)
    {
        syslog(LOG_WARNING, "[I2C] Invalid coordinates (%.6f,%.6f), skipping\n",
               msg->latitude, msg->longitude);
        return 0;
    }

    const time_t msg_time = msg->utc_sec;
    
    if (msg_time < MIN_UTC || msg_time > time(NULL))
    {
        syslog(LOG_WARNING, "[I2C] Invalid time (%s), skipping\n",
               asctime(gmtime(&msg_time)));
        return 0;
    }

    return 1;
}