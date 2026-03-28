#include "Monitor.h"
#include "TimeoutException.h"
#include "timespec.h"

#include <pthread.h>
#include <errno.h>

Monitor::Monitor(Mutex& mutex)
    : mutex(mutex)
{
    pthread_cond_init(&posixCondId, nullptr);
}

Monitor::~Monitor()
{
    pthread_cond_destroy(&posixCondId);
}

void Monitor::notify()
{
    pthread_cond_signal(&posixCondId);
}

void Monitor::notifyAll()
{
    pthread_cond_broadcast(&posixCondId);
}

Monitor::Lock::Lock(Monitor& monitor)
    : Mutex::Lock(monitor.mutex), monitor(monitor)
{
}

Monitor::Lock::Lock(Monitor& monitor, long timeout_ms)
    : Mutex::Lock(monitor.mutex, timeout_ms), monitor(monitor)
{
}

void Monitor::Lock::wait()
{
    pthread_cond_wait(&monitor.posixCondId, &monitor.mutex.posixMutexId);
}

bool Monitor::Lock::wait(long timeout_ms)
{
    timespec ts = timespec_now() + timespec_from_ms(timeout_ms);

    int status = pthread_cond_timedwait(
        &monitor.posixCondId,
        &monitor.mutex.posixMutexId,
        &ts
    );

    if (status == 0)
        return true;

    if (status == ETIMEDOUT)
        return false;

    return false;
}

void Monitor::Lock::notify()
{
    monitor.notify();
}

void Monitor::Lock::notifyAll()
{
    monitor.notifyAll();
}