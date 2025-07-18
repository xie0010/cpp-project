#include "threadPool.h"
threadPool::threadPool(int threadPoolSize):stop(false)
{
    startThreadPool(threadPoolSize);
}

threadPool::~threadPool()
{
    {
        unique_lock<mutex> lock(task_mutex);
        stop = true;
    }
    task_cv.notify_all();
    for(auto &worker:workers)
    {
        worker.join();
    }
}

void threadPool::addTask(function<void()>task)
{
    {
        unique_lock <mutex> lock(task_mutex);
        tasks.push(task);
    }
    task_cv.notify_one();
}

void threadPool::startThreadPool(size_t numThreads)
{
    for(size_t i = 0; i < numThreads; ++i)
    {
        workers.emplace_back([this]{
            while(true)
            {
                function<void()> task;
                {
                    unique_lock<mutex> lock(task_mutex);
                    task_cv.wait(lock,[this]{
                        return stop || !tasks.empty();
                    });

                    if(stop && tasks.empty())
                    {
                        return;
                    }

                    task = move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}