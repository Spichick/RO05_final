#pragma once
#include <memory>
#include "Mutex.h"

class Counter {
public:
    explicit Counter(bool protect);
    ~Counter() = default;

    Counter(const Counter&) = delete;
    Counter& operator=(const Counter&) = delete;

    void increment();
    double value() const;
    bool isProtected() const;

private:
    using IncrFn = void (Counter::*)();

    void incrDoProtect();
    void incrNoProtect();

private:
    double m_value_;
    std::unique_ptr<Mutex> p_mutex_;
    IncrFn incrFn_;
};