#include "tftp_server.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

#define ERR_LOG(msg) do { \
    perror(msg); \
    cout << __LINE__ << " " << __func__ << " " << __FILE__ << endl; \
} while(0)

TFTPServer::TFTPServer(const string &root) : root_dir(root) 
{
    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0) 
    {
        ERR_LOG("socket error");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int reuse = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) 
    {
        ERR_LOG("setsockopt error");
        return;
    }

    if (bind(sfd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
    {
        ERR_LOG("bind error");
        return;
    }
}

TFTPServer::~TFTPServer() {
    if (sfd > 0) {
        close(sfd);
    }
}

void TFTPServer::run() 
{
    cout << "TFTP Server started on port " << PORT << endl;
    cout << "Serving files from " << root_dir << endl;

    char buf[BUFFER_SIZE] = "";
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (true) 
    {
        int n = recvfrom(sfd, buf, BUFFER_SIZE, 0, (sockaddr*)&client_addr, &addr_len);
        if (n < 0) 
        {
            ERR_LOG("recvfrom error");
            return;
        }

        // 检查TFTP请求格式
        if (buf[0] != 0) continue;

        char *filename = buf + 2;
        const char *mode = filename + strlen(filename) + 1;
        if (strcasecmp(mode, "octet") != 0) 
        {
            sendError("Only binary mode supported", client_addr);
            continue;
        }

        switch (buf[1]) 
        {
            case 1:
                cout << "Read request for: " << filename << endl;
                handleReadRequest(filename, client_addr, addr_len);
                break;
            case 2:
                cout << "Write request for: " << filename << endl;
                handleWriteRequest(filename, client_addr, addr_len);
                break;
            default:
                sendError("Unknown request", client_addr);
                break;
        }
    }
}

void TFTPServer::handleReadRequest(const char *filename, struct sockaddr_in &client_addr, socklen_t addr_len) 
{
    string full_path = root_dir + "/" + filename;
    int fd = open(full_path.c_str(), O_RDONLY);
    if (fd < 0) 
    {
        sendError("File not found", client_addr);
        return;
    }

    char buf[BUFFER_SIZE] = "";
    unsigned short block_num = 1;

    while (true) 
    {
        buf[0] = 0;
        buf[1] = 3;  
        *(unsigned short*)(buf + 2) = htons(block_num);
        
        int n = read(fd, buf + 4, BUFFER_SIZE - 4);
        if (n < 0) 
        {
            sendError("Read error", client_addr);
            close(fd);
            return;
        }

        if (sendto(sfd, buf, n + 4, 0, (sockaddr*)&client_addr, addr_len) < 0) 
        {
            ERR_LOG("sendto error");
            close(fd);
            return;
        }

        // 等待ACK
        do 
        {
            if (recvfrom(sfd, buf, BUFFER_SIZE, 0, (sockaddr*)&client_addr, &addr_len) < 0) 
            {
                ERR_LOG("recvfrom error");
                close(fd);
                return;
            }
        } while (buf[1] != 4 || ntohs(*(unsigned short*)(buf + 2)) != block_num);

        // 如果读取的数据少于最大块大小，说明文件传输完成
        if (n < BUFFER_SIZE - 4) 
        {
            break;
        }
        block_num++;
    }
    close(fd);
}

void TFTPServer::handleWriteRequest(const char *filename, struct sockaddr_in &client_addr, socklen_t addr_len) 
{
    string full_path = root_dir + "/" + filename;
    int fd = open(full_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (fd < 0) 
    {
        sendError("Cannot create file", client_addr);
        return;
    }

    char buf[BUFFER_SIZE] = "";
    unsigned short block_num = 0;

    buf[0] = 0;
    buf[1] = 4;  
    *(unsigned short*)(buf + 2) = htons(block_num);

    if (sendto(sfd, buf, 4, 0, (sockaddr*)&client_addr, addr_len) < 0) 
    {
        ERR_LOG("sendto error");
        close(fd);
        return;
    }

    while (true) 
    {
        int n = recvfrom(sfd, buf, BUFFER_SIZE, 0, (sockaddr*)&client_addr, &addr_len);
        if (n < 0) 
        {
            ERR_LOG("recvfrom error");
            close(fd);
            return;
        }

        if (buf[1] == 3 && ntohs(*(unsigned short*)(buf + 2)) == block_num + 1) 
        {
            if (write(fd, buf + 4, n - 4) < 0) 
            {
                sendError("Write error", client_addr);
                close(fd);
                return;
            }

            block_num++;
            buf[0] = 0;
            buf[1] = 4;  
            *(unsigned short*)(buf + 2) = htons(block_num);
            
            if (sendto(sfd, buf, 4, 0, (sockaddr*)&client_addr, addr_len) < 0) 
            {
                ERR_LOG("sendto error");
                close(fd);
                return;
            }

            // 如果接收的数据少于最大块大小，说明文件传输完成
            if (n < BUFFER_SIZE) 
            {
                break;
            }
        }
    }
    close(fd);
}

void TFTPServer::sendError(const char *msg, struct sockaddr_in &client_addr) 
{
    char buf[BUFFER_SIZE] = "";

    buf[0] = 0;
    buf[1] = 5;  
    buf[2] = 0;
    buf[3] = 1;  
    strcpy(buf + 4, msg);

    if (sendto(sfd, buf, strlen(msg) + 5, 0, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0) 
    {
        ERR_LOG("sendto error");
        return;
    }
}