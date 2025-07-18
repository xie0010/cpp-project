#include <vector>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <thread>
using namespace std;
class threadPool
{
private:
    vector<thread> workers;
    queue<function<void()>> tasks;
    condition_variable task_cv;
    mutex task_mutex;
    bool stop;

    void startThreadPool(size_t numThreads);

public:
    threadPool(int threadPoolSize);
    ~threadPool();

    void addTask(function<void()> task);
};

