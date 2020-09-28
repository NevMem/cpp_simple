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

#ifdef DISPATCHER_LOGGING
std::mutex cerrMutex_;

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

    unsigned long long timeSinceCreation() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - packCreated_).count();
    }

private:
    const std::unique_ptr<BasePack> pack_;
    const std::chrono::time_point<std::chrono::high_resolution_clock> packCreated_;
};

#endif

inline void log(const std::string& tag, const std::string& message)
{
#ifdef DISPATCHER_LOGGING
    std::lock_guard<std::mutex> guard(cerrMutex_);
    std::cerr << "[" << tag << "] " << message << std::endl;
#endif
}

class UnstableDispatcher : public Dispatcher {
public:
    virtual std::string name() const override
    {
        return "unstable-dispatcher";
    }

    virtual bool hasTasks() override
    {
        return false; // Unstable!!!
    }

protected:
    virtual void dispatch(std::unique_ptr<BasePack>&& pack) override
    {
        std::thread thread([pack = std::move(pack)]() {
            (*pack)();
        });
        thread.detach();
    }
};

class SingleThreadDispatcher : public Dispatcher {
public:
    SingleThreadDispatcher()
    {
        thread_ = std::thread([this]() {
            while (true) run();
        });
    }

    virtual std::string name() const override
    {
        return "single-thread-dispatcher";
    }

    virtual bool hasTasks() override
    {
        std::lock_guard<std::mutex> guard(queueAccessMutex_);
        return !queue_.empty() || isRunning_;
    }

protected:
    virtual void dispatch(std::unique_ptr<BasePack>&& pack) override
    {
        std::lock_guard<std::mutex> guard(queueAccessMutex_);
        queue_.push(std::forward<std::unique_ptr<BasePack>>(pack));
    }

private:
    void run()
    {
        std::unique_ptr<BasePack> pack = nullptr;
        {
            std::lock_guard<std::mutex> guard(queueAccessMutex_);
            if (!queue_.empty()) {
                pack = std::move(queue_.front());
                queue_.pop();
                isRunning_ = true;
            }
        }
        if (pack) {
            (*pack)();
            {
                std::lock_guard<std::mutex> guard(queueAccessMutex_); // TODO: mutex for isRunning_ (or atomic)
                isRunning_ = false;
            }
        }
    }

    std::queue<std::unique_ptr<BasePack>> queue_;
    std::mutex queueAccessMutex_;
    std::thread thread_;
    bool isRunning_ = false;
};

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

    virtual bool hasTasks() override
    {
        std::lock_guard<std::mutex> guard(mutex_);
        return !queue_.empty() || busyThreads_ != 0;
    }

protected:
    virtual void dispatch(std::unique_ptr<BasePack>&& pack) override
    {
        {
            log("dispatch", "Adding pack");
            std::lock_guard<std::mutex> guard(mutex_);
#ifdef DISPATCHER_LOGGING
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
#ifdef DISPATCHER_LOGGING
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
                    std::unique_lock<std::mutex> lock(mutex_);
                    cv_.wait(lock, [this] { return !queue_.empty(); });
                    pack = std::move(queue_.front());
                    queue_.pop();
                    busyThreads_ += 1;
                }
                if (pack) {
#ifdef DISPATCHER_LOGGING
                    log("run", "Executing pack from queue (time in queue: " + std::to_string(pack->timeSinceCreation()) + " ms)");
#endif
                    (*pack)();
                    log("run", "Done");
                    {
                        std::lock_guard<std::mutex> guard(mutex_);
                        busyThreads_ -= 1;
                    }
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
    }

#ifdef DISPATCHER_LOGGING
    std::queue<QueuePackType> queue_;
#else
    std::queue<QueuePackType> queue_;
#endif
    std::mutex mutex_;
    std::condition_variable cv_;
    size_t busyThreads_ = 0;

    std::vector<std::thread> threads_;
};

}

void initializeDispatchers()
{
    singleton::singleton<UnstableDispatcher>();
    singleton::singleton<SingleThreadDispatcher>();
    singleton::singleton<ThreadPoolDispatcher<4>, 0>();
    singleton::singleton<ThreadPoolDispatcher<16>, 1>();
}

Dispatcher* unstable()
{
    return &singleton::singleton<UnstableDispatcher>();
}

Dispatcher* single()
{
    return &singleton::singleton<SingleThreadDispatcher>();
}

Dispatcher* io()
{
    return &singleton::singleton<ThreadPoolDispatcher<4>, 0>();
}

Dispatcher* computation()
{
    return &singleton::singleton<ThreadPoolDispatcher<16>, 1>();
}

}