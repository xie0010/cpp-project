#include <iostream>
#include "Connection.h"
#include "ConnectionPool.h"

int main()
{
    Connection c;
    c.connect("127.0.0.1", 3306, "root", "0000", "students");
    char sql[1024] = { 0 };
    sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
        "zhang san", 20, "male");
    c.update(sql);
}