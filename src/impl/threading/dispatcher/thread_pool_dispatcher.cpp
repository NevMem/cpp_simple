#include <threading/dispatcher/dispatcher.h>

#include <singleton/singleton.h>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#ifdef DISPATCHER_LOGGING
#include <iostream>
#include <string>
#endif

namespace threading::dispatcher {

namespace {

#ifdef DISPATCHER_EXTENSIONS
#undef DISPATCHER_EXTENSIONS
#endif

#ifdef DISPATCHER_LOGGING
#define DISPATCHER_EXTENSIONS
#endif

#ifdef DISPATCHER_PROFILING
#define DISPATCHER_EXTENSIONS
#endif

#ifdef DISPATCHER_EXTENSIONS
std::mutex cerrMutex_;
#endif

inline void log(const std::string& tag, const std::string& message)
{
#ifdef DISPATCHER_LOGGING
    std::lock_guard<std::mutex> guard(cerrMutex_);
    std::cerr << "[" << tag << "] " << message << std::endl;
#endif
}

inline void logProfile(const std::string& message)
{
#ifdef DISPATCHER_PROFILING
    std::lock_guard<std::mutex> guard(cerrMutex_);
    std::cerr << "<profile> " << message << std::endl;
#endif
}

#ifdef DISPATCHER_EXTENSIONS

class PackWrapper {
public:
    PackWrapper(std::unique_ptr<BasePack>&& pack)
    : pack_(std::forward<std::unique_ptr<BasePack>>(pack))
    , packCreated_(std::chrono::high_resolution_clock::now())
    {}

    PackWrapper()
    : pack_(nullptr)
    , packCreated_(std::chrono::high_resolution_clock::now())
    {}

    void operator ()()
    {
        (*pack_)();
    }

    operator bool() const
    {
        return static_cast<bool>(pack_);
    }

    auto timeSinceCreation() const
    {
        const auto diff = std::chrono::high_resolution_clock::now() - packCreated_;
        return std::chrono::duration<long long, std::nano>(diff);
    }

private:
    const std::unique_ptr<BasePack> pack_;
    const std::chrono::time_point<std::chrono::high_resolution_clock> packCreated_;
};
#endif

template <size_t T>
class ThreadPoolDispatcher : public Dispatcher {
public:
    ThreadPoolDispatcher()
    {
        initializeThreads();
    }

    virtual std::string name() const override
    {
        return "thread-pool-dispatcher-with-" + std::to_string(T) + "-threads";
    }

    void onDestroy()
    {
#ifdef DISPATCHER_PROFILING
        profilingRunning_ = false;
        profilingThread_.join();
        profile();
#endif
    }

protected:
    virtual void dispatch(std::unique_ptr<BasePack>&& pack) override
    {
        {
            log("dispatch", "Adding pack");
            std::lock_guard<std::mutex> guard(mutex_);
#ifdef DISPATCHER_EXTENSIONS
            queue_.push(std::make_unique<PackWrapper>(
                std::forward<std::unique_ptr<BasePack>>(pack)));
            log("queue", "Tasks count: " + std::to_string(queue_.size()));
#else
            queue_.push(std::forward<std::unique_ptr<BasePack>>(pack));
#endif
            log("dispatch", "Added pack");
        }
        cv_.notify_one();
    }

private:
#ifdef DISPATCHER_EXTENSIONS
    typedef std::unique_ptr<PackWrapper> QueuePackType;
#else
    typedef std::unique_ptr<BasePack> QueuePackType;
#endif

    std::thread makeThread()
    {
        return std::thread([this]() {
            while (true) {
                QueuePackType pack;
                {
                    std::lock_guard<std::mutex> guard(mutex_);
                    if (!queue_.empty()) {
                        pack = std::move(queue_.front());
                        queue_.pop();
#ifdef DISPATCHER_EXTENSIONS
                        busyThreads_ += 1;
#endif
                    }
                }
                if (!pack) {
                    std::unique_lock<std::mutex> lock(mutex_);
                    cv_.wait(lock, [this] { return !queue_.empty(); });
                    pack = std::move(queue_.front());
                    queue_.pop();
#ifdef DISPATCHER_EXTENSIONS
                        busyThreads_ += 1;
#endif
                }
                if (pack) {
#ifdef DISPATCHER_EXTENSIONS
                    log("run", "Executing pack from queue (time in queue: " + std::to_string(pack->timeSinceCreation().count()) + " ns)");
                    totalInQueueTime_ += pack->timeSinceCreation();
#endif
#ifdef DISPATCHER_LOGGING
                    const auto start = std::chrono::high_resolution_clock::now();
#endif
                    (*pack)();
#ifdef DISPATCHER_LOGGING
                    const auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count();
                    log("run", "Done in " + std::to_string(diff));
#endif
#ifdef DISPATCHER_EXTENSIONS
                    {
                        std::lock_guard<std::mutex> guard(mutex_);
                        busyThreads_ -= 1;
                    }
#endif
#ifdef DISPATCHER_PROFILING
                    {
                        std::lock_guard<std::mutex> guard(profilingMutex_);
                        dispatchedPacks_ += 1;
                    }
#endif
                }
            }
        });
    }

    void initializeThreads()
    {
        log("init", "Initializing threads");
        threads_.reserve(T);
        for (size_t i = 0; i != T; ++i) {
            threads_.push_back(makeThread());
        }
        log("init", "Initialized " + std::to_string(T) + " threads");

#ifdef DISPATCHER_PROFILING
        profilingThread_ = std::thread([this]() {
            while (profilingRunning_) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                profile();
            }
        });
#endif
    }

#ifdef DISPATCHER_PROFILING
    void profile() const {
        logProfile("Dispatched " + std::to_string(dispatchedPacks_)
            + ", total time in queue: " + std::to_string(totalInQueueTime_.count())
            + ", average latency: " + std::to_string(totalInQueueTime_.count() / std::max(static_cast<size_t>(1), dispatchedPacks_)));
    }

    std::thread profilingThread_;
    bool profilingRunning_ = true;

    mutable std::mutex profilingMutex_;
#endif

#ifdef DISPATCHER_EXTENSIONS
    std::queue<QueuePackType> queue_;
    std::chrono::duration<long long, std::nano> totalInQueueTime_ = std::chrono::duration<long long, std::nano>(0);
    size_t dispatchedPacks_ = 0;
    size_t busyThreads_ = 0;
#else
    std::queue<QueuePackType> queue_;
#endif
    std::mutex mutex_;
    std::condition_variable cv_;

    std::vector<std::thread> threads_;
};

}

Dispatcher* io()
{
    return &singleton::singleton<ThreadPoolDispatcher<4>, 0>();
}

Dispatcher* computation()
{
    return &singleton::singleton<ThreadPoolDispatcher<8>, 1>();
}

void beforeDestroy()
{
    if (const auto dispatcher = dynamic_cast<ThreadPoolDispatcher<8>*>(computation())) {
        dispatcher->onDestroy();
    }
}

}