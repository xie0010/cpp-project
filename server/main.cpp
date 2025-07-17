#include "tftp_server.h"
#include <iostream>
int main(int argc, const char* argv[])
{
    try
    {
        {
            std::string root_dir = (argc>1) ? argv[1] : ".";
            TFTPServer server(root_dir);
            server.run();
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return -1;
    }
    return 0;
}