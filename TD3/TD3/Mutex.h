#pragma once
#include <pthread.h>
#include <stdexcept>
#include <string>
#include "TimeoutException.h"

class Mutex {
public:
    class Lock;

    explicit Mutex(bool isInversionSafe = false);
    ~Mutex();

    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

private:
    pthread_mutex_t posixMutexId_;

    void lock();
    bool lock(double timeout_ms);
    void unlock();

    friend class Lock;
};

class Mutex::Lock {
public:
    explicit Lock(Mutex& m);
    Lock(Mutex& m, long timeout_ms);
    ~Lock() noexcept;

    Lock(const Lock&) = delete;
    Lock& operator=(const Lock&) = delete;

    Lock(Lock&& other) noexcept;
    Lock& operator=(Lock&& other) noexcept;

private:
    Mutex* m_;
    bool locked_;
};