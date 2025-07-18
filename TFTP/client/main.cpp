#include "tftp_client.h"
#include <iostream>
using namespace std;

int main(int argc, char const *argv[])
{
    if(argc < 2)
    {
        cout << "请输入IP地址" << endl;
        return -1;
    }
    try
    {
        TFTPClient Client(argv[1]);
        Client.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return -1;
    }
    return 0;
}