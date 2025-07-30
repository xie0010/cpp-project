#include "database_manager.hpp"
#include <memory>
#include <netinet/in.h>

class Server{
public:
    Server(shared_ptr<DatabaseManager> db_manager, const string ip, int port);
    ~Server();

    bool start();
    bool stop();

private:
    int sfd_;
    int port_;
    string ip_;
    bool running_;
    shared_ptr<DatabaseManager> db_manager_;

    void handleClient(int cfd, sockaddr_in client_addr);

    static string getCurrentTime();
};