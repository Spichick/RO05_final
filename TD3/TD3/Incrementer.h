#pragma once
#include "Thread.h"
#include "Counter.h"
#include "Chrono.h"

class Incrementer : public Thread {
public:
    Incrementer(int id, Counter& counter, double nLoops);

    long startOffsetMs() const;
    void setMainT0(timespec t0);

protected:
    void run() override;

private:
    Counter& r_counter_;
    const double nLoops_;

    timespec mainT0_{};
    timespec myStart_{};
    bool hasStart_{false};
};