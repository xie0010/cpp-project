# pragma once
#include <mysql/mysql.h>
#include <string>
#include <ctime>

class Connection
{
public:
    Connection();
    ~Connection();
    bool connect(std::string ip,
    unsigned short port,
    std::string user,
    std::string password,
    std::string dbname);
    bool update(std::string sql);
    MYSQL_RES* query(std::string sql);
    void refreshAliveTime(){_alivetime = clock();}
    clock_t getAliveTime()const {return clock() - _alivetime;}
private:
    MYSQL *_conn;
    clock_t _alivetime;
};