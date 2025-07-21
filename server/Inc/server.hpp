#pragma once

#include "db.hpp"
#include <cstdint>
#include <netinet/in.h>
#include <string>

namespace Parksys
{
    /**
     * @brief Request types
     */
    enum class ReqType : uint8_t
    {
        IDLE = 0,             // Vehicle is idle
        START = 1,            // Parking start
        STOP = 2              // Parking stop
    };

    struct Request
    {
        ReqType type;         // Request type
        uint32_t license_id;  // Vehicle's license ID
        uint32_t timestamp;   // UTC timestamp
        float latitude;       // GPS latitude
        float longitude;      // GPS longitude
    };

    class Server
    {
    public:
        /**
         * @brief Construct a new Server object
         * 
         * @param ip Server's IP
         * @param port Server's port
         * @param pdb Parksys database object for logging
         * 
         * @throw std::runtime_error when server initialization failed
         */
        Server(const std::string &ip, uint16_t port, Parksys::Database *pdb);

        /**
         * @brief Destroy the Server object
         * 
         */
        ~Server();

        /**
         * @brief Run Parksys server
         * 
         */
        void run();

    private:
        int listen_fd;            // Listening port file descriptor
        sockaddr_in server_addr;  // Server addr struct
        Parksys::Database *pdb;   // Parksys database

        /**
         * @brief Handles a single client 
         * 
         * @param client_fd Client's file descriptor
         */
        void handle_client(int client_fd);

        /**
         * @brief Receives raw request from client
         * 
         * @param fd Client's file descriptor
         * @param buf Buffer to copy request to
         * @param size Buffer's size
         * @return true when successful.
         * @return false otherwise.
         */
        bool recv_request(int fd, void *buf, size_t size);

        /**
         * @brief Parses raw request into request struct
         * 
         * @param buf Buffer containing raw request
         * @param req Request struct to load data to
         * @return true when successful.
         * @return false otherwise.
         */
        bool parse_request(const uint8_t *buf, Parksys::Request &req);

        /**
         * @brief Handles a request according to what was requested
         * 
         * @param req Request struct to handle
         */
        void handle_request(const Parksys::Request &req);
    };
}