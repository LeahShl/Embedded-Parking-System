#include "gps_msg.h"

const char *type_str(uint8_t t)
{
    switch (t)
    {
        case MSGT_IDLE: return "IDLE";
        case MSGT_START: return "START";
        case MSGT_STOP: return "STOP";
        default: return "UNKNOWN";
    }
}