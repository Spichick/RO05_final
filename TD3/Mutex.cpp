#include "Mutex.h"
#include "TimespecUtils.h"
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

static std::runtime_error posix_error(const std::string& where, int err) {
    return std::runtime_error(where + ": " + std::string(std::strerror(err)));
}

Mutex::Mutex(bool isInversionSafe) : posixMutexId_{} {
    int err = 0;

    if (!isInversionSafe) {
        err = pthread_mutex_init(&posixMutexId_, nullptr);
        if (err != 0) throw posix_error("pthread_mutex_init(default)", err);
        return;
    }

    pthread_mutexattr_t attr;
    err = pthread_mutexattr_init(&attr);
    if (err != 0) throw posix_error("pthread_mutexattr_init", err);

    err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    if (err != 0) {
        pthread_mutexattr_destroy(&attr);
        throw posix_error("pthread_mutexattr_settype(PTHREAD_MUTEX_RECURSIVE)", err);
    }

#ifdef PTHREAD_PRIO_INHERIT
    err = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    if (err != 0) {
        pthread_mutexattr_destroy(&attr);
        throw posix_error("pthread_mutexattr_setprotocol(PTHREAD_PRIO_INHERIT)", err);
    }
#else
    pthread_mutexattr_destroy(&attr);
    throw std::runtime_error("PTHREAD_PRIO_INHERIT not supported on this platform");
#endif

    err = pthread_mutex_init(&posixMutexId_, &attr);
    pthread_mutexattr_destroy(&attr);
    if (err != 0) throw posix_error("pthread_mutex_init(inversion-safe)", err);
}

Mutex::~Mutex() {
    pthread_mutex_destroy(&posixMutexId_);
}

void Mutex::lock() {
    const int err = pthread_mutex_lock(&posixMutexId_);
    if (err != 0) throw posix_error("pthread_mutex_lock", err);
}

bool Mutex::lock(double timeout_ms) {
    timespec absTs = TimespecUtils::add_ms(TimespecUtils::now_realtime(), timeout_ms);

    const int err = pthread_mutex_timedlock(&posixMutexId_, &absTs);
    if (err == 0) return true;
    if (err == ETIMEDOUT) return false;

    throw posix_error("pthread_mutex_timedlock", err);
}

void Mutex::unlock() {
    const int err = pthread_mutex_unlock(&posixMutexId_);
    if (err != 0) throw posix_error("pthread_mutex_unlock", err);
}

Mutex::Lock::Lock(Mutex& m) : m_(&m), locked_(false) {
    m_->lock();
    locked_ = true;
}

Mutex::Lock::Lock(Mutex& m, long timeout_ms) : m_(&m), locked_(false) {
    const bool ok = m_->lock(timeout_ms);
    if (!ok) {
        m_ = nullptr;
        throw TimeoutException(timeout_ms);
    }
    locked_ = true;
}

Mutex::Lock::~Lock() noexcept {
    if (m_ && locked_) {
        (void)pthread_mutex_unlock(&m_->posixMutexId_);
    }
}

Mutex::Lock::Lock(Lock&& other) noexcept : m_(other.m_), locked_(other.locked_) {
    other.m_ = nullptr;
    other.locked_ = false;
}

Mutex::Lock& Mutex::Lock::operator=(Lock&& other) noexcept {
    if (this == &other) return *this;

    if (m_ && locked_) {
        (void)pthread_mutex_unlock(&m_->posixMutexId_);
    }

    m_ = other.m_;
    locked_ = other.locked_;
    other.m_ = nullptr;
    other.locked_ = false;
    return *this;
}