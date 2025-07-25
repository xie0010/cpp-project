#include <thread>
#include <string>
#include <cstring>
using namespace std;
#define N 128
#define LOGIN 1
#define CHAT 2
#define QUIT 3

class chatClient
{
private:
    int cfd;
    string name;
    bool running;

    void errLog(const char* msg);
    void sendMsg(int type, const string &text = "");
    void recvMsg();
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
    chatClient(const char* ip, int port, const string &name);

    ~chatClient();

    void run();
};