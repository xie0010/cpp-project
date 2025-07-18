#pragma once
#include <error.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
class TFTPClient
{
private:
    static const int PORT = 8888;
    static const int BUFFER_SIZE = 516;

    int sfd;
    struct sockaddr_in server_addr;

    int doDownload();
    int doUpload();
    void waitForInput();
    void clearScreen(); 
    void showMenu();

public:
    TFTPClient(const std::string &serverIp);
    ~TFTPClient();

    void run();
};