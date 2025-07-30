#include "server.hpp"
#include "message.hpp"
#include <time.h>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>

Server::Server(shared_ptr<DatabaseManager> db_manager, const string ip, int port):
    db_manager_(db_manager), ip_(ip), port_(port), running_(false)
{
    sfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd_ < 0){
        throw runtime_error("socket error");
    }

    int opt = 1;
    if(setsockopt(sfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt) < 0)){
        close(sfd_);
        throw runtime_error("setsockopt error");
    }

    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(ip.c_str());

    if(bind(sfd_, (sockaddr *)&sin, sizeof(sin)) < 0){
        close(sfd_);
        throw runtime_error("bind error");
    }
}

Server::~Server(){
    stop();
}

bool Server::start(){
    if(listen(sfd_, 5) < 0){
        cerr << "listen error" << endl;
        return false;
    }

    running_ = true;
    cout << "Serve started on" << ip_ << ":" << port_ << endl;

    while(running_){
        sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        int cfd = accept(sfd_, (struct sockaddr*)&client_addr, &addr_len);
        if(cfd < 0){
            if(running_){
                cerr << "accept error" << endl;
            }
            continue;
        }

        thread([this, cfd, client_addr]{
            cout << "Client connected:" << inet_ntoa(client_addr.sin_addr) 
                << ":" << ntohs(client_addr.sin_port) << endl;
            handleClient(cfd, client_addr);
        }).detach();
    }
    return true;
}

bool Server::stop()
{
    if(running_){
        running_ = false;
        close(sfd_);
        cout << "Server stopped" << endl;
        return true;
    }
    return false;
}

void Server::handleClient(int cfd, sockaddr_in client_addr){
    Msg msg;
    while(true){
        ssize_t recv_len = recv(cfd, &msg, sizeof(msg), 0);
        if(recv_len <= 0){
            if(recv_len == 0){
                cout << "Client disconnect:" << inet_ntoa(client_addr.sin_addr) << endl;
            }else{
                cerr << "recv error" << endl;
            }
            break;
        }
        msg.hostByteOrder();
        Msg response;
        strncpy(response.name, msg.name, sizeof(response.name));
        response.type = msg.type;

        switch(msg.type){
            case R:
            {
                int success = db_manager_->registerUser(msg.name,msg.text);
                strcpy(response.text, success ? "OK" : "EXISTS");
                break;
            }
            case L:
            {
                bool is_online = false;
                bool success = db_manager_->loginUser(msg.name, msg.text, is_online);
                if(success){
                    strcpy(response.text, is_online ? "EXISTS" : "OK");
                }else{
                    strcpy(response.text, "FAIL");
                }
                break;
            }
            case Q:
            {
                db_manager_->logoutUser(msg.name);
                close(cfd);
                break;
            }
            case S:
            {
                string meaning;
                if(db_manager_->querryWord(msg.text, meaning)){
                    snprintf(response.text, sizeof(response.text),"%s %s", 
                            msg.text, meaning.c_str());
                }else{
                    strcpy(response.text, "Not Found");
                }

                time_t now = time(NULL);
                tm *local = localtime(&now);

                char time_str[20] = "";
                strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",local);
                db_manager_->recordHistory(msg.name, msg.text, meaning, time_str);
                break;
            }
            case H:
            {
                string history;
                if(db_manager_->getHistory(msg.name, history)){
                    strncpy(response.text, history.c_str(), sizeof(response.text));
                }else{
                    strcpy(response.text, "No history");
                }
                break;
            }
            default:strcpy(response.text, "Ivlid command");
        }

        response.networkByteOrder();
        if(send(cfd, &response, sizeof(response), 0) < 0){
            cerr << "send error" << endl;
            break;
        }

        if(msg.type == H){
            strcpy(response.text, "OVER");
            send(cfd, &response, sizeof(response), 0);
        }
    }

    close(cfd);
}

string Server::getCurrentTime(){
    time_t now = time(NULL);
    tm *local = localtime(&now);
    char time_str[20] = "";
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",local);
    return string (time_str);
}