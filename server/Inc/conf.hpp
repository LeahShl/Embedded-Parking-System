#pragma once

#define DB_PATH "parksys/parksys.db"   // Database path relative to user's home folder
#define SHM_PATH "/dev/shm/parksys.db" // Database shared memory path

#define SERVER_IP "0.0.0.0"            // Don't change that unless you know what you're doing
#define SERVER_PORT 12321              // Server's listening port
#define REQ_QUEUE_SIZE 8               // Request queue size for listen()