#include "chatServer.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>

chatServer::chatServer(const char* ip, int port, size_t threadPoolsize):stop(false)
{
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd < 0)
    {
        perror("socket error");
        return;
    }

    int reuse = 1;
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        perror("setsockopt error");
        return;
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(ip);

    if(bind(sfd, (struct sockaddr*)&sin, sizeof(sin)) < 0)
    {
        perror("bind error");
        return;
    }

    if(listen(sfd, 10) < 0)
    {
        perror("listen error");
        return;
    }

    startThreadPool(threadPoolsize);
}

chatServer::~chatServer()
{
    stop = true;
    task_cv.notify_all();
    for(auto &worker:workers)
    {
        worker.join();
    }
    close(sfd);
}

void chatServer::startThreadPool(size_t numThreads)
{
    for(size_t i = 0; i < numThreads; ++i)
    {
        workers.emplace_back([this]
        {
            while(true)
            {
                function<void()> task;
                {
                    unique_lock<mutex> lock(task_mutex);
                    task_cv.wait(lock, [this]{
                        return stop || !tasks.empty();
                    });
                    if(stop && tasks.empty())
                    {
                        return;
                    }
                    task = move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}

void chatServer::addTask(function<void()> task)
{
    {
        unique_lock<mutex> lock(task_mutex);
        tasks.push(task);
    }
    task_cv.notify_one();
}

void chatServer::errLog(const char* msg)
{
    cerr << __FILE__ << " " << __func__ << " " << __LINE__ << endl;
    perror(msg);
}

void chatServer::run()
{
    while(true)
    {
        struct sockaddr_in cin;
        socklen_t clen = sizeof(cin);
        int client_fd = accept(sfd, (struct sockaddr*)&cin, &clen);
        if(client_fd < 0)
        {
            errLog("accept error");
            continue;
        }
        addTask([this, client_fd, cin]{
            handleclient(client_fd, cin);
        });
    }
}

void chatServer::handleclient(int client_fd, struct sockaddr_in cin)
{
    MSG msg;
    char buffer[sizeof(MSG)];
    while(true)
    {
        int recv_len = recv(client_fd, buffer, sizeof(buffer), 0);
        if(recv_len <= 0)
        {
            unique_lock<mutex> lock(client_mutex);
            auto it = clients.begin();
            while(it != clients.end())
            {
                if(it->fd == client_fd)
                {
                    it = clients.erase(it);
                    break;
                }
                ++it;
            }
            close(client_fd);
            break;
        }
        msg.deserialize(string(buffer,recv_len));
        switch(ntohl(msg.type))
        {
        case LOGIN:
            {
                unique_lock<mutex> lock(client_mutex);
                Client new_client;
                new_client.fd = client_fd;
                new_client.cin = cin;
                clients.push_back(new_client);
                sprintf(msg.text, "-------%s 登录成功-----------",msg.name);
                broadcast(msg);
                break;
            }
        case CHAT:
            {
                unique_lock<mutex> lock(client_mutex);
                broadcast(msg, client_fd);
                break;
            }
        case QUIT:
            {
                unique_lock<mutex> lock(client_mutex);
                auto it = clients.begin();
                while(it != clients.end())
                {
                    if(it->fd == client_fd)
                    {
                        sprintf(msg.text, "---------%s 退出聊天室---------",msg.name);
                        broadcast(msg);
                        clients.erase(it);
                        break;
                    }
                    ++it;
                }
                close(client_fd);
                break;
            }
        default:
            cout << "消息类型有误" << endl;
            break;
        }
    }
}
void chatServer::broadcast(const MSG& msg, int exclude_fd)
{
    string data = msg.serialize();
    for(const auto &client:clients)
    {
        if(client.fd != exclude_fd)
        {
            if(send(client.fd, data.c_str(), data.size(), 0) < 0)
            {
                perror("send error");
                continue;
            }
        }
    }
}