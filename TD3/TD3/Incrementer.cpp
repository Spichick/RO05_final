#include "Incrementer.h"

Incrementer::Incrementer(int id, Counter& counter, double nLoops)
    : Thread(id), r_counter_(counter), nLoops_(nLoops) {}

void Incrementer::setMainT0(timespec t0) {
    mainT0_ = t0;
}

void Incrementer::run() {
    myStart_ = Chrono::now();
    hasStart_ = true;

    const long long loops = (nLoops_ < 0.0) ? 0LL : static_cast<long long>(nLoops_);
    for (long long i = 0; i < loops; ++i) {
        r_counter_.increment();
    }
}

long Incrementer::startOffsetMs() const {
    if (!hasStart_) return 0;
    return Chrono::diff_ms(mainT0_, myStart_);
}