#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <limits>
#include "Mutex.h"
#include "Monitor.h"

class Semaphore
{
public:
    using CountType = unsigned long;

private:
    volatile CountType counter;
    CountType maxCount;
    Mutex mutex;
    Monitor notEmpty;

public:
    explicit Semaphore(
        CountType initValue = 0,
        CountType maxValue = std::numeric_limits<CountType>::max()
    );

    void give();
    void take();
    bool take(long timeout_ms);
};

#endif