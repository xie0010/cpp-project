#include "tftp_client.h"
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
using namespace std;
#define ERR_LOG(msg) do{\
    perror(msg);\
    cout << __LINE__ << " " << __func__ << " " << __FILE__ << endl;\
}while(0)

TFTPClient::TFTPClient(const string &serverIp)
{
    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sfd < 0)
    {
        ERR_LOG("sock error");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(serverIp.c_str());
}

TFTPClient::~TFTPClient()
{
    if(sfd > 0)
    {
        close(sfd);
    }
}

void TFTPClient::run()
{
    while(true)
    {
        showMenu();

        char choice;
        cin >> choice;
        waitForInput();

        switch(choice)
        {
        case '1':
            doDownload();
            break;
        case '2':
            doUpload();
            break;
        default:
            cout << "输入有误请重新输入" << endl;
            break;
        }

        clearScreen();
    }
}

int TFTPClient::doDownload()
{
    string filename;
    cout << "请输入要下载的文件名称:";
    getline(cin, filename);

    char buf[BUFFER_SIZE] = "";
    int size = sprintf(buf, "%c%c%s%c%s%c", 0, 1, filename.c_str(), 0, "octet", 0);

    if(sendto(sfd, buf, size, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        ERR_LOG("server_addr");
        return -1;
    }

    size_t recv_len;
    unsigned short num = 1;
    socklen_t addrlen = sizeof(server_addr);
    int flag = 0;
    int fd;

    while(true)
    {
        bzero(buf, BUFFER_SIZE);
        
        recv_len = recvfrom(sfd, buf, BUFFER_SIZE, 0, (sockaddr*)&server_addr, &addrlen);
        if(recv_len < 0)
        {
            ERR_LOG("recvfrom error");
            return -1;
        }

        if(3 == buf[1])
        {
            if(0 == flag)
            {
                fd = open(filename.c_str(), O_WRONLY|O_CREAT|O_TRUNC,0064);
                if(fd < 0)
                {
                    ERR_LOG("open error");
                    return -1;
                }
                flag = 1;
            }
            if(htons(num) == *(unsigned short*)(buf+2))
            {
                if(write(fd, buf+4, recv_len -4) < 0)
                {
                    cout << "fd = " << fd << " recv_len = " << recv_len << endl;
                    ERR_LOG("write error");
                    close(fd);
                    break;
                }
            }
            buf[1] = 4;
            if(sendto(sfd, buf, 4, 0, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
            {
                ERR_LOG("send to");
                close(fd);
                return -1;
            }
            if(recv_len < BUFFER_SIZE)
            {
                cout << "------------文件下载完成--------------" << endl;
                close(fd);
                break;
            }
            num++;
        }
        else if(5 == buf[1])
        {
            cout << "________error: " << (buf+4) << "________" << endl;
            if(flag == 1)
            {
                close(fd);
            }
            break;
        }
    }
    return 0;
}

int TFTPClient::doUpload()
{
    string filename;
    cout << "请输入要上传的文件名：";
    getline(cin, filename);

    int fd = open(filename.c_str(), O_RDONLY);

    if(fd < 0)
    {
        if(errno == ENOENT)
        {
            cout << "文件不存在" << endl;
            return -1;
        }
    }

    char buf[BUFFER_SIZE] = "";
    int size = sprintf(buf, "%c%c%s%c%s%c", 0, 2, filename.c_str(), 0, "octet", 0);
    if(sendto(sfd, buf, size, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        ERR_LOG("sendto error");
        close(fd);
        return -1;
    }

    int recv_len;
    unsigned short num = 0;
    socklen_t addrlen = sizeof(server_addr);

    while(true) {
        recv_len = recvfrom(sfd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, &addrlen);
        if(recv_len < 0)
        {
            ERR_LOG("recvfrom error");
            close(fd);
            return -1;
        }

        if(4 == buf[1])
        {
            if(num == ntohs(*(unsigned short*)(buf+2)))
            {
                buf[1] = 3;
                num++;
                *(unsigned short*)(buf+2) = htons(num);
            }

            int res = read(fd, buf+4, BUFFER_SIZE-4);
            if(res < 0)
            {
                ERR_LOG("read error");
                close(fd);
                break;
            }
            else if(res == 0)
            {
                cout << "--------------上传完成------------" << endl;
                break;
            }
            if(sendto(sfd, buf, res+4, 0, (struct sockaddr*)&server_addr,sizeof(server_addr)) < 0)
            {
                ERR_LOG("sendto error");
                close(fd);
                return-1;
            }
        }
        else if(5 == buf[1])
        {
            cout << "-----------文件上传失败-----------" << endl;
            break;
        }
    }
    return 0;
}

void TFTPClient::clearScreen()
{
    cout<<"请输入任意字符进行清屏";
    while(getchar()!='\n');
}

void TFTPClient::waitForInput()
{
    while(getchar()!='\n');
}

void TFTPClient::showMenu()
{
    
    system("clear");
    cout<<"******************基于UDP的TFTP文件传输********************"<<endl;
    cout<<"*********************1、下载************************"<<endl;
    cout<<"*********************2、上传************************"<<endl;
    cout<<"*********************3、退出************************"<<endl;
    cout<<"**********************************************************"<<endl;
}