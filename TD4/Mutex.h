#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>
#include "TimeoutException.h"
#include "timespec.h"

class Monitor;

class Mutex
{
public:
    class Lock;

private:
    pthread_mutex_t posixMutexId;
    pthread_mutexattr_t posixMutexAttrId;
    bool inversionSafe;

    friend class Monitor;

public:
    explicit Mutex(bool isInversionSafe = false);
    ~Mutex();

private:
    void lock();
    bool lock(double timeout_ms);
    void unlock();
};

class Mutex::Lock
{
private:
    Mutex& mutex;

public:
    Lock(Mutex& mutex);
    Lock(Mutex& mutex, double timeout_ms);
    ~Lock();
};

#endif