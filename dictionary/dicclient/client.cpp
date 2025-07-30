#include "client.hpp"
#include <iostream>
#include <cstring>
using namespace std;
void DictClient::run(){
    while(true){
        showMainMenu();
        int choice;
        cin >> choice;
        cin.ignore();

        switch (choice){
            case R:
                doRegister();
                break;
            case L:
                if(doLogin()){
                    showUserMenu();
                }
                break;
            case Q:
                doQuit();
                break;
            default: cout << "无效的选择" << endl;
            break;
        }
        cout << "按回车继续...";
        cin.get();
    }
}

void DictClient::showMainMenu(){
    system("clear");         
    cout<<"***************************"<<endl;
    cout<<"*********1、注册***********"<<endl;
    cout<<"*********2、登录***********"<<endl;
    cout<<"*********3、退出***********"<<endl;
    cout<<"***************************"<<endl;
    cout<<"请选择：";
}

void DictClient::showUserMenu(){
    while(is_logged_in_){
        system("clear");           
        cout<<"当前用户："<<username_<<endl;
        cout<<"*********************************"<<endl;
        cout<<"**********1、查单词***************"<<endl;
        cout<<"**********2、历史记录***************"<<endl;
        cout<<"**********3、返回上一级***************"<<endl;
        cout<<"*********************************"<<endl;
        cout<<"请选择：";

        int choice;
        cin >> choice;
        cin.ignore();

        switch (choice){
            case 1:
                doQuerry();
                break;
            case 2:
                doHistory();
                break;
            case 3:
                doQuit();
                break;
            default: cout << "无效的选择" << endl;
            break;
        }
        cout << "按回车继续...";
        cin.get();
    }
}

bool DictClient::doRegister(){
    Msg msg;
    msg.type = R;

    cout << "请输入用户名称（最长19个字符）：";
    cin.getline(msg.text, sizeof(msg.text));
    cout << "请输入密码（最长127个字符）：";
    cin.getline(msg.text, sizeof(msg.text));

    msg.networkByteOrder();

    if(send(sockfd_, &msg, sizeof(msg), 0) < 0){
        perror("注册请求失败");
        return false;
    }

    if(recv(sockfd_, &msg, sizeof(msg), 0) < 0){
        perror("接收注册信息失败");
        return false;
    }
    msg.hostByteOrder();

    if(strcmp(msg.text, "OK") == 0){
        cout  << "注册成功" << endl;
        return true;
    }else if(strcmp(msg.text, "EXISTS") == 0){
        cout << "注册失败，用户名已经存在" << endl;
    }else{
        cout << "注册失败，错误未知" << endl;
    }
    return false;
}

bool DictClient::doLogin(){
    Msg msg;
    msg.type = L;

    cout << "请输入用户名：";
    cin.getline(msg.name, sizeof(msg.name));
    cout << "请输入密码：";
    cin.getline(msg.text, sizeof(msg.text));

    msg.networkByteOrder();
    if(send(sockfd_, &msg, sizeof(msg), 0) < 0){
        perror("发送登录请求失败");
        return false;
    }

    if(recv(sockfd_, &msg, sizeof(msg), 0) < 0){
        perror("接收登录响应失败");
        return false;
    }

    msg.hostByteOrder();

    if(strcmp(msg.text, "OK") == 0){
        cout << "登录成功" << endl;
        username_ = msg.name;
        is_logged_in_ = true;

        return true;
    }else if (strcmp(msg.text, "OK")){
        cout << "登录失败，用户已经在线" << endl;
    }else{
        cout << "登录失败，原因未知" << endl;
    }

    return false;
}

void DictClient::doQuerry(){
    Msg msg;
    msg.type = S;
    strcpy(msg.name, username_.c_str());

    while(true){
        cout << "请输入要查询的单词（输入#后结束查询）：";
        cin.getline(msg.text, sizeof(msg.text));

        if(strcmp(msg.text, "#") == 0){
            cout << "------------------";
            break;
        }

        msg.networkByteOrder();
        if(send(sockfd_, &msg, sizeof(msg), 0) < 0){
            perror("发送查询消息失败");
            return;
        }

        if(recv(sockfd_, &msg, sizeof(msg), 0) < 0){
            perror("接收查询结果失败");
            return;
        }
        msg.hostByteOrder();
        cout << "释义：" << msg.text << endl;
    }
}

void DictClient::doHistory(){
    Msg msg;
    msg.type = H;
    strcpy(msg.name, username_.c_str());

    msg.networkByteOrder();
    if(send(sockfd_, &msg, sizeof(msg), 0) < 0){
        perror("发送历史请求失败");
        return;
    }
    cout << "查询历史记录" << endl;
    while(true){
        if(recv(sockfd_, &msg, sizeof(msg), 0) <= 0){
            perror("接收历史数据失败");
            return;
        }
        msg.hostByteOrder();
        if(strcmp(msg.text, "OVER") == 0){
            break;
        }

        cout << msg.text << endl;
    }
}

void DictClient::doQuit(){
    if(is_logged_in_){
        Msg msg;
        msg.type = Q;
        strcpy(msg.name, username_.c_str());
        msg.networkByteOrder();
        if(send(sockfd_, &msg, sizeof(msg), 0) < 0){
            perror("发送请求失败");
            return;
        }else{
            is_logged_in_ = false;
            username_.clear();
            cout << "用户已经退出" << endl;
        }
    }
   
}