#include <threading/dispatcher/dispatcher.h>

#include <singleton/singleton.h>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace threading::dispatcher {

namespace {

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
            std::lock_guard<std::mutex> guard(mutex_);
            queue_.push(std::forward<std::unique_ptr<BasePack>>(pack));
        }
        cv_.notify_one();
    }

private:
    std::thread makeThread()
    {
        return std::thread([this]() {
            while (true) {
                std::unique_ptr<BasePack> pack;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    cv_.wait(lock, [this] { return !queue_.empty(); });
                    pack = std::move(queue_.front());
                    queue_.pop();
                    busyThreads_ += 1;
                }
                if (pack) {
                    (*pack)();
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
        threads_.reserve(T);
        for (size_t i = 0; i != T; ++i) {
            threads_.push_back(makeThread());
        }
    }

    std::queue<std::unique_ptr<BasePack>> queue_;
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
    singleton::singleton<ThreadPoolDispatcher<4>, 1>();
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
    return &singleton::singleton<ThreadPoolDispatcher<8>, 1>();
}

}