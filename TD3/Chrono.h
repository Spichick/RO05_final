#pragma once
#include <ctime>

class Chrono {
public:
    Chrono() : t0_(now()) {}

    void restart() { t0_ = now(); }

    double seconds() const {
        const timespec t1 = now();
        long sec  = static_cast<long>(t1.tv_sec - t0_.tv_sec);
        long nsec = static_cast<long>(t1.tv_nsec - t0_.tv_nsec);
        if (nsec < 0) { sec -= 1; nsec += 1000000000L; }
        return static_cast<double>(sec) + static_cast<double>(nsec) / 1e9;
    }

    static timespec now() {
        timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts;
    }

    static long diff_ms(const timespec& a, const timespec& b) {
        long sec  = static_cast<long>(b.tv_sec - a.tv_sec);
        long nsec = static_cast<long>(b.tv_nsec - a.tv_nsec);
        if (nsec < 0) { sec -= 1; nsec += 1000000000L; }
        return sec * 1000L + nsec / 1000000L;
    }

private:
    timespec t0_;
};