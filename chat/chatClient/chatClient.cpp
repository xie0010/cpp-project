#include "chatClient.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
chatClient::chatClient(const char *ip, int port, const string &name):name(name)
{
    cfd = socket(AF_INET, SOCK_STREAM, 0);
    if(cfd < 0)
    {
        perror("socket error");
        return;
    }
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(ip);
    
    if(connect(cfd, (struct sockaddr*)&sin, sizeof(sin)) < 0)
    {
        perror("connect error");
        return;
    }
    sendMsg(LOGIN);
}

chatClient::~chatClient()
{
    sendMsg(QUIT);
    close(cfd);
}

void chatClient::run()
{
    thread recvThread(&chatClient::recvMsg, this);
    string text;
    while(running)
    {
        getline(cin,text);
        sendMsg(CHAT, text);
    }
    recvThread.join();
}

void chatClient::errLog(const char* msg)
{
    cerr << __FILE__ << " " << __func__ << " " << __LINE__ << endl;
    perror(msg);
}

void chatClient::sendMsg(int type, const string &text)
{
    MSG msg;
    msg.type = htonl(type);
    strncpy(msg.name, name.c_str(), sizeof(msg.name));
    strncpy(msg.text, text.c_str(),sizeof(msg.text));
    msg.name[sizeof(msg.name) - 1] = '\0';
    string data = msg.serialize();
    if(send(cfd, data.c_str(), data.size(), 0) < 0)
    {
        errLog("send error");
        return;
    }
    cout << "消息发送成功" << endl;
    if(text == "quit")
    {
        running = false;
        exit(EXIT_SUCCESS);
    }
}

void chatClient::recvMsg()
{
    char buffer[sizeof(MSG)];
    while(running)
    {
        int recv_len = recv(cfd, buffer, sizeof(buffer), 0);
        if(recv_len <= 0)
        {
            errLog("recv error");
            running = false;
            break;
        }
        MSG msg;
        msg.deserialize(string(buffer, recv_len));
        cout << msg.name << ": " << msg.text << endl;
    }
}