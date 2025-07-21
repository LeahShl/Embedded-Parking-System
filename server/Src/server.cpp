#include "server.hpp"
#include "conf.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <thread>

Parksys::Server::Server(const std::string &ip, uint16_t port, Parksys::Database *pdb)
: pdb(pdb)
{
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        throw std::runtime_error("Failed to create socket");
    }

    // enable socket reuse
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        close(listen_fd);
        throw std::runtime_error("Invalid IP address: " + ip);
    }

    if (bind(listen_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        perror("bind");
        close(listen_fd);
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(listen_fd, REQ_QUEUE_SIZE) < 0) {
        perror("listen");
        close(listen_fd);
        throw std::runtime_error("Failed to listen");
    }

    std::cout << "[Server] Listening on TCP " << ip << ":" << port << std::endl;
}

Parksys::Server::~Server()
{
    close(listen_fd);
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
            perror("accept");
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
    constexpr size_t req_SIZE = 1 + 4 + 4 + 4 + 4;
    uint8_t buffer[req_SIZE];

    while (true)
    {
        if (!recv_request(client_fd, buffer, req_SIZE))
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
            std::cerr << "[Server] Received invalid message" << std::endl;
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
    req.type = static_cast<ReqType>(buf[offset]);
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
    switch (req.type) {
    case ReqType::START:
        std::cout << "[Server] | " << req.timestamp
                  << " | START recorded for license " << req.license_id
                  << " at (" << req.latitude << "," << req.longitude << ")"
                  << std::endl;
        break;

    case ReqType::STOP:
        std::cout << "[Server] | " << req.timestamp
                  << " | STOP recorded for license " << req.license_id
                  << " at (" << req.latitude << "," << req.longitude << ")"
                  << std::endl;
        break;

    default:
        std::cerr << "[Server] Unsupported message type" << std::endl;
        break;
    }
}