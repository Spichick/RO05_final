#pragma once

#include <time.h>

class Chrono {
private:
    timespec m_startTime{};
    timespec m_stopTime{};
    bool     m_active{false};

public:
    Chrono();

    timespec restart();
    timespec stop();
    bool isActive() const;

    timespec lap() const;
    double lap_ms() const;
};
