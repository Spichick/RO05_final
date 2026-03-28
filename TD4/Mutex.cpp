#include "Mutex.h"
#include <errno.h>

Mutex::Mutex(bool isInversionSafe)
    : inversionSafe(isInversionSafe)
{
    pthread_mutexattr_init(&posixMutexAttrId);

    if (inversionSafe)
    {
        pthread_mutexattr_setprotocol(&posixMutexAttrId, PTHREAD_PRIO_INHERIT);
    }

    pthread_mutex_init(&posixMutexId, &posixMutexAttrId);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&posixMutexId);
    pthread_mutexattr_destroy(&posixMutexAttrId);
}

void Mutex::lock()
{
    pthread_mutex_lock(&posixMutexId);
}

bool Mutex::lock(double timeout_ms)
{
    timespec ts = timespec_now() + timespec_from_ms(timeout_ms);

    int status = pthread_mutex_timedlock(&posixMutexId, &ts);

    if (status == 0)
        return true;

    if (status == ETIMEDOUT)
        throw TimeoutException(static_cast<long>(timeout_ms));

    return false;
}

void Mutex::unlock()
{
    pthread_mutex_unlock(&posixMutexId);
}

Mutex::Lock::Lock(Mutex& mutex)
    : mutex(mutex)
{
    this->mutex.lock();
}

Mutex::Lock::Lock(Mutex& mutex, double timeout_ms)
    : mutex(mutex)
{
    this->mutex.lock(timeout_ms);
}

Mutex::Lock::~Lock()
{
    mutex.unlock();
}