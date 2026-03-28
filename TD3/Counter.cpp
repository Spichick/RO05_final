#include "Counter.h"

Counter::Counter(bool protect)
    : m_value_(0.0),
      p_mutex_(protect ? std::make_unique<Mutex>(false) : nullptr),
      incrFn_(protect ? &Counter::incrDoProtect : &Counter::incrNoProtect)
{}

bool Counter::isProtected() const {
    return static_cast<bool>(p_mutex_);
}

void Counter::increment() {
    (this->*incrFn_)();
}

void Counter::incrDoProtect() {
    Mutex::Lock lock(*p_mutex_);
    m_value_ += 1.0;
}

void Counter::incrNoProtect() {
    m_value_ += 1.0;
}

double Counter::value() const {
    return m_value_;
}