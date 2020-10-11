#include <threading/dispatcher/dispatcher.h>

#include <singleton/singleton.h>

#include <thread>

namespace threading::dispatcher {

namespace {

class UnstableDispatcher : public Dispatcher {
public:
    virtual std::string name() const override
    {
        return "unstable-dispatcher";
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

}

Dispatcher* unstable()
{
    return &singleton::singleton<UnstableDispatcher>();
}

}
