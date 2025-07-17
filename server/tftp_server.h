#pragma once
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
class TFTPServer
{
private:
    static const int PORT = 8888;
    static const int BUFFER_SIZE = 516;

    int sfd;
    struct sockaddr_in server_addr;
    std::string root_dir;

    void handleReadRequest(const char *filename, struct sockaddr_in &client_addr, socklen_t addr_len);
    void handleWriteRequest(const char *filename, struct sockaddr_in &client_addr, socklen_t addr_len);
    void sendError(const char *msg, struct sockaddr_in &client_addr);

public:
    TFTPServer(const std::string &root = ".");
    ~TFTPServer();

    void run();
};