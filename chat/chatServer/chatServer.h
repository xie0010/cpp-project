#include <vector>
#include <string>
#include <thread>
#include <queue>
#include <condition_variable>
#include <functional>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;
#define N 128
#define LOGIN 1
#define CHAT 2
#define QUIT 3
class chatServer
{
public:
    struct MSG{
        int type;
        char name[20];
        char text[N];

        string serialize() const
        {
            string data;
            data.append((reinterpret_cast<const char*>(&type)),sizeof(type));     
            data.append(name,sizeof(name));
            data.append(text,sizeof(text));
            return data;
        }

        void deserialize(const string &data)
        {
            size_t offset = 0;
            memcpy(&type, data.c_str()+offset,sizeof(type));
            offset += sizeof(type);
            memcpy(name, data.c_str()+offset,sizeof(name));
            offset += sizeof(name);
            memcpy(text, data.c_str()+offset,sizeof(text));
        }
    };

    typedef struct Client{
        int fd;
        struct sockaddr_in cin;
    }Client;

    void run();
    void handleclient(int client_fd, struct sockaddr_in cin);
    void broadcast(const MSG& msg, int exclude_fd = -1);

    chatServer(const char* ip, int port, size_t threadPoolsize = 4);
    ~chatServer();

private:
    int sfd;
    vector<Client> clients;
    mutex client_mutex;
    vector<thread> workers;
    queue<function<void()>> tasks;
    mutex task_mutex;
    condition_variable task_cv;
    bool stop;

    void errLog(const char *msg);
    void startThreadPool(size_t numThreads);
    void addTask(function<void()> task);

};

