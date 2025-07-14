/**
 * @file main.c
 * @author Leah
 * @brief Parking system project file for BBG
 * @date 2025-07-14
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <string.h>

#define I2C_BUS        "/dev/i2c-1"    // I2C bus must be added as system's overlay
#define I2C_ADDR       0x10            // STM32 I2C slave address
#define MSG_LEN        17              // 1 + 4 + 4 + 4 + 4 bytes

typedef struct __attribute__((packed))
{
    uint8_t  msg_type;                 // 0=idle, 1=start, 2=stop
    uint32_t license_id;               // 0-99999999
    uint32_t utc_sec;                  // seconds since reference
    float    latitude;                 // degrees
    float    longitude;                // degrees
} gps_msg_t;

const char *type_str(uint8_t t)
{
    switch (t)
    {
        case 0: return "IDLE";
        case 1: return "START";
        case 2: return "STOP";
        default: return "UNKNOWN";
    }
}

int main(void)
{
    int fd = open(I2C_BUS, O_RDWR);
    if (fd < 0)
    {
        perror("open " I2C_BUS);
        return 1;
    }

    if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0)
    {
        perror("ioctl I2C_SLAVE");
        close(fd);
        return 1;
    }

    printf("[I2C] Listening on %s @0x%02X ...\n", I2C_BUS, I2C_ADDR);

    while (1)
    {
        uint8_t buf[MSG_LEN];
        ssize_t rd = read(fd, buf, MSG_LEN);

        if (rd < 0)
        {
            if (errno == EAGAIN || errno == EINTR || errno == EBUSY || errno == EIO || errno == ENXIO)
            {
                fprintf(stderr, "[I2C] Warning: read failed (%s), retrying...\n", strerror(errno));
                usleep(500000); 
                continue;
            }
            continue;
        }

        if (rd != MSG_LEN)
        {
            fprintf(stderr, "[I2C] short read: %zd/%d bytes, retrying...\n", rd, MSG_LEN);
            usleep(100000);
            continue;
        }

        gps_msg_t msg;
        memcpy(&msg, buf, sizeof(msg));

        if (msg.msg_type > 2)
        {
            fprintf(stderr, "[I2C] Invalid msg_type %u, skipping\n", msg.msg_type);
            continue;
        }

        printf("[I2C] msg_type=%s(%u), license=%08u, utc_sec=%u, lat=%.6f, lon=%.6f\n",
               type_str(msg.msg_type),
               msg.msg_type,
               msg.license_id,
               msg.utc_sec,
               msg.latitude,
               msg.longitude);

        sleep(1);
    }

    close(fd);
    return 0;
}
