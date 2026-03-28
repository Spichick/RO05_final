#include <iostream>
#include "Monitor.h"
#include "Mutex.h"

int main()
{
    Mutex mutex;
    Monitor monitor(mutex);

    {
        Monitor::Lock lock(monitor);
        lock.notify();
        lock.notifyAll();
    }

    std::cout << "OK" << std::endl;
    return 0;
}