#include "client.hpp"
#include <iostream>

int main(int argc, const char* argv[]){
    if(argc != 3){
        cerr << "用法:" << argv[0] << " <服务器Ip> <端口号>" << endl;
        return -1;
    }

    try{
        DictClient client(argv[1], atoi(argv[2]));
        client.run();
    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }

    return 0;
}