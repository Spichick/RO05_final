#include "Semaphore.h"
#include "timespec.h"

#include <stdexcept>

Semaphore::Semaphore(CountType initValue, CountType maxValue)
    : counter(initValue),
      maxCount(maxValue),
      mutex(),
      notEmpty(mutex)
{
    if (maxValue < 1)
    {
        throw std::invalid_argument("Semaphore maxValue must be >= 1");
    }

    if (initValue > maxValue)
    {
        throw std::invalid_argument("Semaphore initValue must be <= maxValue");
    }
}

void Semaphore::give()
{
    Monitor::Lock lock(notEmpty);

    if (counter < maxCount)
    {
        ++counter;
        lock.notify();
    }
}

void Semaphore::take()
{
    Monitor::Lock lock(notEmpty);

    while (counter == 0)
    {
        lock.wait();
    }

    --counter;
}

bool Semaphore::take(long timeout_ms)
{
    Monitor::Lock lock(notEmpty);

    if (counter > 0)
    {
        --counter;
        return true;
    }

    timespec deadline = timespec_now() + timespec_from_ms(timeout_ms);

    while (counter == 0)
    {
        timespec now = timespec_now();

        if (deadline < now || deadline == now)
        {
            return false;
        }

        long remaining_ms = static_cast<long>(timespec_to_ms(deadline - now));
        if (remaining_ms <= 0)
        {
            return false;
        }

        if (!lock.wait(remaining_ms))
        {
            return false;
        }
    }

    --counter;
    return true;
}