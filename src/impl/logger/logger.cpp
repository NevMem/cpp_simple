#include <logger/logger.h>

#include <cassert>
#include <fstream>
#include <mutex>

namespace logger {

namespace {

static std::shared_ptr<Logger> loggerHolder;

class LoggerImpl : public Logger {
public:
    LoggerImpl(const std::string& logFileName)
    : outputStream_(logFileName + ".log")
    {}

    virtual void log(const std::string& message) override
    {
        if (!isLoggingEnabled_) {
            return;
        }
        std::lock_guard<std::mutex> guard(streamMutex_);
        outputStream_ << message << "\n";
    }

    virtual void setLogingEnabled(bool isEnabled) override
    {
        isLoggingEnabled_ = isEnabled;
    }

    virtual ~LoggerImpl()
    {
        outputStream_.flush();
        outputStream_.close();
    }

private:
    std::ofstream outputStream_;
    std::mutex streamMutex_;
    bool isLoggingEnabled_ = true;
};

}

void initializeLogger(const std::string& logFileName)
{
    assert(!loggerHolder); // Should not reinitialize logger
    loggerHolder = std::make_shared<LoggerImpl>(logFileName);
}

Logger* logger()
{
    assert(loggerHolder);
    return loggerHolder.get();
}

}
