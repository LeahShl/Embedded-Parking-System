#include "gps_msg.h"

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