#include "server.hpp"
#include "conf.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <thread>

Parksys::Server::Server(const std::string &ip, uint16_t port, Parksys::Database *pdb)
: pdb(pdb),
log(std::string(std::getenv("HOME")) + "/" + LOG_PATH),
err(std::string(std::getenv("HOME")) + "/" + ERR_PATH)
{
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        err.threadsafe_log("[SERVER] Failed to create socket");
        throw std::runtime_error("Failed to create socket");
    }

    // enable socket reuse
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // set timeout
    struct timeval timeout;
    timeout.tv_sec = SERV_TIMO_SEC;
    timeout.tv_usec = SERV_TIMO_MS;
    setsockopt(listen_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(listen_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));


    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        close(listen_fd);
        err.threadsafe_log(std::string("[SERVER] Invalid IP address: ") + ip);
        throw std::runtime_error("Invalid IP address: " + ip);
    }

    if (bind(listen_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        close(listen_fd);
        err.threadsafe_log("[SERVER] Failed to bind socket");
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(listen_fd, REQ_QUEUE_SIZE) < 0) {
        close(listen_fd);
        err.threadsafe_log("[SERVER] Failed to listen");
        throw std::runtime_error("Failed to listen");
    }

    log.threadsafe_log("[Server] Listening on TCP " + ip + ":" + std::to_string(port));
}

Parksys::Server::~Server()
{
    close(listen_fd);

    // Release shared memory since the server runs no longer
    std::remove(SHM_PATH);
}

void Parksys::Server::run()
{
    while (true)
    {
        sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int client_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&client_addr), &addrlen);
        if (client_fd < 0)
        {
            err.threadsafe_log("[SERVER] Timeout reached");
            continue;
        }

        // Handle each client in a separate thread
        std::thread([this, client_fd]()
        {
            this->handle_client(client_fd);
            close(client_fd);
        }).detach();
    }
}

void Parksys::Server::handle_client(int client_fd)
{
    uint8_t buffer[REQ_SIZE];

    while (true)
    {
        if (!recv_request(client_fd, buffer, REQ_SIZE))
        {
            break;
        }

        Request req;
        if (parse_request(buffer, req))
        {
            handle_request(req);
        }
        else
        {
            err.threadsafe_log("[Server] Received invalid message");
        }
    }
}

bool Parksys::Server::recv_request(int fd, void *buf, size_t size)
{
    size_t total = 0;
    uint8_t* ptr = static_cast<uint8_t*>(buf);
    while (total < size)
    {
        ssize_t bytes = recv(fd, ptr + total, size - total, 0);
        if (bytes <= 0)
        {
            return false;
        }
        total += bytes;
    }
    return true;
}

bool Parksys::Server::parse_request(const uint8_t *buf, Parksys::Request &req)
{
    size_t offset = 0;

    auto raw_type = buf[offset];
    if (raw_type > static_cast<uint8_t>(ReqType::STOP)) {
        err.threadsafe_log("[Server] Invalid ReqType: " + static_cast<int>(raw_type));
        return false;
    }
    req.type = static_cast<ReqType>(raw_type);

    offset += sizeof(req.type);

    std::memcpy(&req.license_id, buf + offset, sizeof(req.license_id));
    offset += sizeof(req.license_id);

    std::memcpy(&req.timestamp, buf + offset, sizeof(req.timestamp));
    offset += sizeof(req.timestamp);

    std::memcpy(&req.latitude, buf + offset, sizeof(req.latitude));
    offset += sizeof(req.latitude);

    std::memcpy(&req.longitude, buf + offset, sizeof(req.longitude));

    return true;
}

void Parksys::Server::handle_request(const Parksys::Request &req)
{
    uint32_t lot_id = 0;
    if (!this->pdb->findClosestLot(req.latitude, req.longitude, lot_id))
    {
        err.threadsafe_log("[Server] No parking lots found in database.");
        return;
    }

    switch (req.type) {
    case ReqType::START:
        if (this->pdb->startParking(lot_id, req.license_id, req.timestamp) != pdbStatus::PDB_OK)
            err.threadsafe_log("[Server] Failed to log START");
        else
            log.threadsafe_log("[Server] | " + std::to_string(req.timestamp) +
                  " | START recorded for license " + std::to_string(req.license_id) +
                  " at (" + std::to_string(req.latitude) + "," + std::to_string(req.longitude) + ") | " +
                  "Lot " + std::to_string(lot_id));
        break;

    case ReqType::STOP:
        if (this->pdb->endParking(req.license_id, req.timestamp) != pdbStatus::PDB_OK)
            err.threadsafe_log("[Server] Failed to log STOP");
        else
            log.threadsafe_log("[Server] | " + std::to_string(req.timestamp) +
                  " | STOP recorded for license " + std::to_string(req.license_id) +
                  " at (" + std::to_string(req.latitude) + "," + std::to_string(req.longitude) + ") | " +
                  "Lot " + std::to_string(lot_id));
        break;

    default:
        err.threadsafe_log("[Server] Unsupported message type");
        break;
    }
}