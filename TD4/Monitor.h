#ifndef MONITOR_H
#define MONITOR_H

#include <pthread.h>
#include "Mutex.h"

class Monitor
{
public:
    class Lock;

private:
    pthread_cond_t posixCondId;
    Mutex& mutex;

public:
    explicit Monitor(Mutex& mutex);
    ~Monitor();

    void notify();
    void notifyAll();

private:
    Monitor(const Monitor&) = delete;
    Monitor& operator=(const Monitor&) = delete;
};

class Monitor::Lock : public Mutex::Lock
{
private:
    Monitor& monitor;

public:
    explicit Lock(Monitor& monitor);
    Lock(Monitor& monitor, long timeout_ms);

    void wait();
    bool wait(long timeout_ms);

    void notify();
    void notifyAll();
};

#endif