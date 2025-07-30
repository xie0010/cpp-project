#include <netinet/in.h>
#define R 1
#define L 2
#define Q 3
#define S 4
#define H 5

struct Msg{
    int type;
    char name[20];
    char text[512];

    void networkByteOrder(){
        type = htonl(type);
    }

    void hostByteOrder(){
        type = ntohl(type);
    }
};