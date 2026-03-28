#pragma once
#include <pthread.h>
#include <ctime>
#include <stdexcept>
#include <string>

class Thread {
private:
    pthread_t      posixThreadId_;
    pthread_attr_t posixThreadAttrId_;
    const int      id_;

    bool started_;
    timespec t_start_;
    timespec t_end_;
    bool hasTiming_;

public:
    explicit Thread(int id);
    virtual ~Thread();

    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    void start(int priority = 0);
    void join();

    bool isStarted() const;
    long duration_ms() const;

protected:
    virtual void run() = 0;

private:
    static void* call_run(void* v_thread);

public:
    static int  getMaxPrio(int policy);
    static int  getMinPrio(int policy);

    static void setMainSched(int policy);
    static int  getMainSched();
};