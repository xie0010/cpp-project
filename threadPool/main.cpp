#include "threadPool.h"
#include <iostream>
using namespace std;

int main()
{
    threadPool threads(3);
    for(int i = 0; i < 20; ++i)
    {
        threads.addTask([i](){
            cout << "任务：" << i << "正在执行,其tid = " << this_thread::get_id() << endl;
            this_thread::sleep_for(std::chrono::milliseconds(500));
        });
    }
    this_thread::sleep_for(std::chrono::seconds(5));

    return 0;
}