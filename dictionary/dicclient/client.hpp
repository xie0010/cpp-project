#include "message.hpp"
#include <string>
using namespace std;
class DictClient
{
private:
    int sockfd_;
    string username_;
    bool is_logged_in_;

    bool connectToserver(const string &ip, int port);

    void showMainMenu();
    void showUserMenu();
    bool doRegister();
    bool doLogin();
    void doQuerry();
    void doHistory();
    void doQuit();
public:
    DictClient(const string &ip, int port);
    ~DictClient();
    void run();
};


