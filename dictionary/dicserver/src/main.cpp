#include "database_manager.hpp"
#include"server.hpp"     
#include <iostream>

int main(int argc, const char *argv[]){
    if(argc != 3){
        cerr << "用法：" << argv[0] << "<ip> <port>" << endl;
        return -1;
    }
    try
    {
        auto db_manager = make_shared<DatabaseManager>("usr.db","dict.db");
        if(!db_manager->initalizeDatabase()){
            cerr<<"数据库初始化失败，请检查："<<endl;
            cerr<<"1、dict.txt文件是否存在"<<endl;
            cerr<<"2、当前目录是否具有写权限"<<endl;
            return -1;
        }
        Server server(db_manager, argv[1], atoi(argv[2]));
        server.start();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return -1;
    }
    
}