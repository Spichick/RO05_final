#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include "Monitor.h"
#include "Mutex.h"

struct SharedData
{
    Monitor* monitor;
    volatile bool* ready;
};

void* thread_func(void* arg)
{
    SharedData* data = static_cast<SharedData*>(arg);

    std::cout << "Thread: waiting..." << std::endl;

    Monitor::Lock lock(*data->monitor);
    while (!(*data->ready))
    {
        lock.wait();
    }

    std::cout << "Thread: awakened!" << std::endl;
    return nullptr;
}

int main()
{
    Mutex mutex;
    Monitor monitor(mutex);
    volatile bool ready = false;

    SharedData data{&monitor, &ready};

    pthread_t th;
    pthread_create(&th, nullptr, thread_func, &data);

    sleep(1);

    {
        Monitor::Lock lock(monitor);
        ready = true;
        lock.notify();
    }

    pthread_join(th, nullptr);

    std::cout << "Main: done." << std::endl;
    return 0;
}