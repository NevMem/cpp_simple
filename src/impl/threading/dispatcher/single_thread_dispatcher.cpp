#include <threading/dispatcher/dispatcher.h>

#include <singleton/singleton.h>

#include <mutex>
#include <queue>
#include <thread>

namespace threading::dispatcher {

namespace {

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
            }
        }
        if (pack) {
            (*pack)();
        }
    }

    std::queue<std::unique_ptr<BasePack>> queue_;
    std::mutex queueAccessMutex_;
    std::thread thread_;
};

}

Dispatcher* single()
{
    return &singleton::singleton<SingleThreadDispatcher>();
}

}