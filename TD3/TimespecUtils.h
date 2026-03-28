#pragma once
#include <ctime>
#include <cerrno>
#include <stdexcept>
#include <string>

namespace TimespecUtils {

inline timespec now_realtime() {
    timespec ts{};
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        throw std::runtime_error("clock_gettime(CLOCK_REALTIME) failed");
    }
    return ts;
}

inline timespec add_ms(timespec t, long timeout_ms) {
    if (timeout_ms < 0) timeout_ms = 0;

    const long sec_add  = timeout_ms / 1000;
    const long nsec_add = (timeout_ms % 1000) * 1000000L;

    t.tv_sec += sec_add;
    t.tv_nsec += nsec_add;

    if (t.tv_nsec >= 1000000000L) {
        t.tv_sec += 1;
        t.tv_nsec -= 1000000000L;
    }
    return t;
}

}