#include "chrono.h"
#include "timespec_utils.h"

Chrono::Chrono() {
    restart(); // spec: at creation, initialized by calling restart()
}

timespec Chrono::restart() {
    m_startTime = timespec_now();
    m_stopTime  = m_startTime;
    m_active = true;
    return m_startTime;
}

timespec Chrono::stop() {
    m_stopTime = timespec_now();
    m_active = false;
    return timespec_subtract(m_stopTime, m_startTime);
}

bool Chrono::isActive() const {
    return m_active;
}

timespec Chrono::lap() const {
    if (m_active) {
        return timespec_subtract(timespec_now(), m_startTime);
    }
    return timespec_subtract(m_stopTime, m_startTime);
}

double Chrono::lap_ms() const {
    return timespec_to_ms(lap());
}
