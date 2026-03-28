#pragma once

#include <time.h>

// Normalize so that tv_nsec in [0, 1e9) even if overall time is negative.
timespec timespec_normalize(timespec t);

// ms <-> timespec
double   timespec_to_ms(const timespec& t);
timespec timespec_from_ms(double ms);

// now / negate
timespec timespec_now();
timespec timespec_negate(const timespec& t);

// add / subtract
timespec timespec_add(const timespec& a, const timespec& b);
timespec timespec_subtract(const timespec& a, const timespec& b);

// wait using nanosleep, must handle EINTR
void timespec_wait(const timespec& delay);

// operators (treat timespec like a scalar)
timespec operator-(const timespec& t);
timespec operator+(const timespec& a, const timespec& b);
timespec operator-(const timespec& a, const timespec& b);
timespec& operator+=(timespec& a, const timespec& b);
timespec& operator-=(timespec& a, const timespec& b);

bool operator==(const timespec& a, const timespec& b);
bool operator!=(const timespec& a, const timespec& b);
bool operator<(const timespec& a, const timespec& b);
bool operator>(const timespec& a, const timespec& b);
