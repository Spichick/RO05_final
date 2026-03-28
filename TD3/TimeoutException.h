#pragma once
#include <exception>
#include <string>

class TimeoutException : public std::exception {
public:
    const long timeout_ms;

    explicit TimeoutException(double timeout_ms_)
        : timeout_ms(timeout_ms_),
          msg_("Timeout after " + std::to_string(timeout_ms) + " ms") {}

    const char* what() const noexcept override { return msg_.c_str(); }

private:
    std::string msg_;
};