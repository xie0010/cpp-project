#include "chatClient.h"
#include <iostream>
int main(int argc, const char *argv[])
{
    if(argc < 4)
    {
        cout << "请输入ip 端口号 用户名" << endl;
    }
    chatClient client(argv[1], atoi(argv[2]), argv[3]);
    client.run();

    return 0;
}