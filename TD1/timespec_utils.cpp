#include "timespec_utils.h"

#include <errno.h>
#include <cmath>

static constexpr long NSEC_PER_SEC = 1000000000L;

timespec timespec_normalize(timespec t) {
    // First, move any extra nanoseconds into seconds.
    if (t.tv_nsec >= NSEC_PER_SEC || t.tv_nsec <= -NSEC_PER_SEC) {
        long long carry = t.tv_nsec / NSEC_PER_SEC;  // trunc toward 0
        t.tv_sec += carry;
        t.tv_nsec -= carry * NSEC_PER_SEC;
    }

    // Ensure tv_nsec is in [0, 1e9)
    if (t.tv_nsec < 0) {
        t.tv_sec -= 1;
        t.tv_nsec += NSEC_PER_SEC;
    }
    return t;
}

double timespec_to_ms(const timespec& t_in) {
    timespec t = timespec_normalize(t_in);
    // tv_sec can be negative; tv_nsec is [0, 1e9)
    return 1000.0 * static_cast<double>(t.tv_sec) + static_cast<double>(t.tv_nsec) / 1e6;
}

timespec timespec_from_ms(double ms) {
    // Convert ms -> sec + nsec, then normalize
    // Use floor to handle negatives correctly.
    double sec_d = std::floor(ms / 1000.0);
    double rem_ms = ms - sec_d * 1000.0;          // in [0,1000) even if ms negative
    long nsec = static_cast<long>(std::llround(rem_ms * 1e6)); // ms->ns

    timespec t{};
    t.tv_sec = static_cast<time_t>(sec_d);
    t.tv_nsec = nsec;

    // nsec could be 1e9 due to rounding
    return timespec_normalize(t);
}

timespec timespec_now() {
    timespec t{};
    // Use monotonic clock for durations/chrono
    clock_gettime(CLOCK_MONOTONIC, &t);
    return timespec_normalize(t);
}

timespec timespec_negate(const timespec& t_in) {
    timespec t = timespec_normalize(t_in);
    // negate while keeping tv_nsec in [0, 1e9)
    if (t.tv_nsec == 0) {
        t.tv_sec = -t.tv_sec;
        return t;
    }
    // Example: -(s + ns) = -(s+1) + (1e9-ns)
    timespec r{};
    r.tv_sec = -t.tv_sec - 1;
    r.tv_nsec = NSEC_PER_SEC - t.tv_nsec;
    return timespec_normalize(r);
}

timespec timespec_add(const timespec& a_in, const timespec& b_in) {
    timespec a = timespec_normalize(a_in);
    timespec b = timespec_normalize(b_in);
    timespec r{};
    r.tv_sec = a.tv_sec + b.tv_sec;
    r.tv_nsec = a.tv_nsec + b.tv_nsec;
    return timespec_normalize(r);
}

timespec timespec_subtract(const timespec& a, const timespec& b) {
    return timespec_add(a, timespec_negate(b));
}

void timespec_wait(const timespec& delay_in) {
    timespec delay = timespec_normalize(delay_in);
    if (delay.tv_sec < 0) return; // ignore negative sleeps

    timespec rem{};
    while (true) {
        int rc = nanosleep(&delay, &rem);
        if (rc == 0) return;              // slept fully
        if (errno == EINTR) {
            delay = rem;                  // sleep remaining time
            continue;
        }
        return; // other error: just return
    }
}

// Operators
timespec operator-(const timespec& t) { return timespec_negate(t); }
timespec operator+(const timespec& a, const timespec& b) { return timespec_add(a, b); }
timespec operator-(const timespec& a, const timespec& b) { return timespec_subtract(a, b); }

timespec& operator+=(timespec& a, const timespec& b) { a = timespec_add(a, b); return a; }
timespec& operator-=(timespec& a, const timespec& b) { a = timespec_subtract(a, b); return a; }

static int cmp_timespec(const timespec& a_in, const timespec& b_in) {
    timespec a = timespec_normalize(a_in);
    timespec b = timespec_normalize(b_in);
    if (a.tv_sec < b.tv_sec) return -1;
    if (a.tv_sec > b.tv_sec) return 1;
    if (a.tv_nsec < b.tv_nsec) return -1;
    if (a.tv_nsec > b.tv_nsec) return 1;
    return 0;
}

bool operator==(const timespec& a, const timespec& b) { return cmp_timespec(a, b) == 0; }
bool operator!=(const timespec& a, const timespec& b) { return cmp_timespec(a, b) != 0; }
bool operator<(const timespec& a, const timespec& b)  { return cmp_timespec(a, b) < 0; }
bool operator>(const timespec& a, const timespec& b)  { return cmp_timespec(a, b) > 0; }
