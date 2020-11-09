#pragma once

#include <memory>
#include <string>

namespace logger {

class Logger {
public:
    virtual ~Logger() = default;

    // Thread-safe function (may lock until resource safely available)
    virtual void log(const std::string& message) = 0;

    virtual void setLogingEnabled(bool isEnabled) = 0;
};

// Will create {logFileName}.log all logs will be stored there
void initializeLogger(const std::string& logFileName);

Logger* logger();

}
