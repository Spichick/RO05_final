#include "Thread.h"
#include <cerrno>
#include <cstring>
#include <sched.h>
#include <unistd.h>

static std::runtime_error posix_error(const std::string& where, int err) {
    return std::runtime_error(where + ": " + std::string(std::strerror(err)));
}

static timespec now_monotonic() {
    timespec ts{};
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        throw std::runtime_error("clock_gettime(CLOCK_MONOTONIC) failed");
    }
    return ts;
}

static long diff_ms(const timespec& a, const timespec& b) {
    long sec  = static_cast<long>(b.tv_sec - a.tv_sec);
    long nsec = static_cast<long>(b.tv_nsec - a.tv_nsec);
    if (nsec < 0) { sec -= 1; nsec += 1000000000L; }
    return sec * 1000L + nsec / 1000000L;
}

Thread::Thread(int id)
    : posixThreadId_{},
      posixThreadAttrId_{},
      id_(id),
      started_(false),
      t_start_{},
      t_end_{},
      hasTiming_(false)
{
    const int err = pthread_attr_init(&posixThreadAttrId_);
    if (err != 0) throw posix_error("pthread_attr_init", err);

}

Thread::~Thread() {
    (void)pthread_attr_destroy(&posixThreadAttrId_);
}

bool Thread::isStarted() const {
    return started_;
}

void Thread::start(int priority) {
    if (started_) {
        throw std::runtime_error("Thread::start: already started");
    }

    const int policy = Thread::getMainSched();

    if (policy == SCHED_OTHER) {
        if (priority != 0) {
            throw std::runtime_error("Thread::start: SCHED_OTHER requires priority = 0");
        }
    } else {
        const int pmin = getMinPrio(policy);
        const int pmax = getMaxPrio(policy);
        if (priority < pmin || priority > pmax) {
            throw std::runtime_error("Thread::start: priority out of range");
        }
    }

    int err = 0;

    err = pthread_attr_setinheritsched(&posixThreadAttrId_, PTHREAD_EXPLICIT_SCHED);
    if (err != 0) throw posix_error("pthread_attr_setinheritsched", err);

    err = pthread_attr_setschedpolicy(&posixThreadAttrId_, policy);
    if (err != 0) throw posix_error("pthread_attr_setschedpolicy", err);

    sched_param sp{};
    sp.sched_priority = priority;

    err = pthread_attr_setschedparam(&posixThreadAttrId_, &sp);
    if (err != 0) throw posix_error("pthread_attr_setschedparam", err);

    err = pthread_create(&posixThreadId_, &posixThreadAttrId_, &Thread::call_run, this);
    if (err != 0) throw posix_error("pthread_create", err);

    started_ = true;
}

void Thread::join() {
    if (!started_) {
        throw std::runtime_error("Thread::join: thread not started");
    }
    const int err = pthread_join(posixThreadId_, nullptr);
    if (err != 0) throw posix_error("pthread_join", err);
}

long Thread::duration_ms() const {
    if (!hasTiming_) {
        throw std::runtime_error("Thread::duration_ms: timing not available yet");
    }
    return diff_ms(t_start_, t_end_);
}

void* Thread::call_run(void* v_thread) {
    Thread* self = static_cast<Thread*>(v_thread);
    self->t_start_ = now_monotonic();
    self->run();
    self->t_end_ = now_monotonic();
    self->hasTiming_ = true;
    return nullptr;
}

int Thread::getMaxPrio(int policy) {
    const int v = sched_get_priority_max(policy);
    if (v == -1) throw posix_error("sched_get_priority_max", errno);
    return v;
}

int Thread::getMinPrio(int policy) {
    const int v = sched_get_priority_min(policy);
    if (v == -1) throw posix_error("sched_get_priority_min", errno);
    return v;
}

void Thread::setMainSched(int policy) {
    sched_param sp{};

    if (policy == SCHED_OTHER) {
        sp.sched_priority = 0;
    }
    else if (policy == SCHED_FIFO || policy == SCHED_RR) {
        sp.sched_priority = getMaxPrio(policy);
    }
    else {
        throw std::runtime_error("Unsupported scheduling policy");
    }

    const int err = pthread_setschedparam(pthread_self(), policy, &sp);
    if (err != 0) {
        throw std::runtime_error("pthread_setschedparam(main) failed");
    }
}

int Thread::getMainSched() {
    int policy = 0;
    sched_param sp{};

    const int err = pthread_getschedparam(pthread_self(), &policy, &sp);
    if (err != 0) {
        throw std::runtime_error("pthread_getschedparam failed");
    }

    return policy;
}