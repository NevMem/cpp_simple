#include <threading/dispatcher/dispatcher.h>
#include <threading/dispatcher/internal/internal.h>

#include <singleton/singleton.h>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace threading::dispatcher {

namespace {

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
                }
                if (pack) {
                    (*pack)();
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

    std::vector<std::thread> threads_;
};

}

Dispatcher* io()
{
    return &singleton::singleton<ThreadPoolDispatcher<internal::IO_POOL_THREADS_COUNT>, 0>();
}

Dispatcher* computation()
{
    return &singleton::singleton<ThreadPoolDispatcher<internal::COMPUTATION_POOL_THREADS_COUNT>, 1>();
}

}