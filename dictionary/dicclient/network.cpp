#include "client.hpp"
#include <stdexcept>
#include <unistd.h>
#include <arpa/inet.h>

DictClient::DictClient(const string &ip, int port):
sockfd_(-1), is_logged_in_(false)
{
    if(!connectToserver(ip, port)){
        throw runtime_error("connect to server error");
    }
}

DictClient::~DictClient()
{
    if(sockfd_ >= 0){
        doQuit();
        close(sockfd_);
    }
}

bool DictClient::connectToserver(const string &ip, int port){
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd_ < 0){
        perror("socket error");
        return false;
    }

    int opt = 1;
    if(setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
        perror("setsockopt error");
        return false;
    }

    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(ip.c_str());

    if(connect(sockfd_, (struct sockaddr*)&sin, sizeof(sin)) < 0){
        perror("connect error");
        return false;
    }

    return true;
}