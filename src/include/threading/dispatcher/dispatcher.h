#pragma once

#include <functional>
#include <string>
#include <future>
#include <iostream>

namespace threading::dispatcher {

namespace {

template <typename F, typename ArgTuple, size_t needArgs, bool Done, size_t... N>
struct FunctorCall {
    auto static call(F functor, ArgTuple& tuple)
    {
        return FunctorCall<F, ArgTuple, needArgs, needArgs == 1 + sizeof...(N), N..., sizeof...(N)>::call(
            functor, tuple);
    }
};

template <typename F, typename ArgTuple, size_t needArgs, size_t... N>
struct FunctorCall<F, ArgTuple, needArgs, true, N...> {
    auto static call(F functor, ArgTuple& tuple)
    {
        return functor(std::get<N>(std::forward<ArgTuple>(tuple))...);
    }
};

template<typename F, typename ArgTuple>
auto functorCall(F functor, ArgTuple& tuple)
{
    return FunctorCall<F, ArgTuple,
        std::tuple_size<typename std::decay<ArgTuple>::type>::value,
        std::tuple_size<typename std::decay<ArgTuple>::type>::value == 0>::call(
            functor, tuple);
}

}

class BasePack {
public:
    virtual ~BasePack() = default;
    virtual void operator()() = 0;
};

template<typename R, typename... Args>
class Pack : public BasePack {
public:
    template<typename F>
    explicit Pack(F&& function, Args&&... args)
    : function_(std::forward<F>(function))
    , args_(std::make_tuple(std::forward<Args>(args)...))
    {}

    virtual void operator()() override
    {
        runFunction();
    }

private:
    R runFunction()
    {
        return functorCall<std::function<R(Args...)>, R, std::tuple<typename std::decay<Args>::type...>>(
            function_, args_);
    }

    const std::function<R(Args...)> function_;
    std::tuple<typename std::decay<Args>::type...> args_;
};

template<typename R, typename... Args>
class FuturePack : public BasePack {
public:
    template<typename F>
    explicit FuturePack(F&& function, Args&&... args)
    : function_(std::forward<F>(function))
    , args_(std::make_tuple(std::forward<Args>(args)...))
    {}

    virtual void operator()() override
    {
        run<R>();
    }

    std::future<R> future()
    {
        return promise_.get_future();
    }

private:
    
    template<typename ResultType>
    typename std::enable_if<!std::is_same<ResultType, void>::value, void>::type run()
    {
        try {
            ResultType result = runFunction();
            promise_.set_value(result);
        } catch (...) {
            promise_.set_exception(std::current_exception()); // Can throw exception TODO: handle that
        }
    }

    template<typename ResultType>
    typename std::enable_if<std::is_same<ResultType, void>::value, void>::type run()
    {
        try {
            runFunction();
            promise_.set_value();
        } catch (...) {
            promise_.set_exception(std::current_exception()); // Can throw exception TODO: handle that
        }
    }

    R runFunction()
    {
        return functorCall(function_, args_);
    }

    const std::function<R(typename std::decay<Args>::type...)> function_;
    std::tuple<typename std::decay<Args>::type...> args_;
    std::promise<R> promise_;
};

class Dispatcher {
public:
    template<typename F, typename... Args>
    void spawn(F&& func, Args&&... args) {
        auto pack = std::unique_ptr<Pack<typename std::decay<typename std::result_of<F(Args...)>::type>::type, Args...>>(new Pack<typename std::decay<typename std::result_of<F(Args...)>::type>::type, Args...>(
            std::forward<F>(func),
            std::forward<Args>(args)...));
        dispatch(std::move(pack));
    }

    template<typename F, typename... Args>
    std::future<typename std::decay<typename std::result_of<F(Args...)>::type>::type> async(F&& func, Args&&... args) {
        typedef typename std::decay<typename std::result_of<F(Args...)>::type>::type ReturnType;
        auto pack = std::unique_ptr<FuturePack<ReturnType, Args...>>(new FuturePack<ReturnType, Args...>(
            std::forward<F>(func),
            std::forward<Args>(args)...));
        auto future = pack->future();
        dispatch(std::move(pack));
        return future;
    }

    virtual std::string name() const = 0;

    virtual bool hasTasks() = 0;

protected:
    virtual void dispatch(std::unique_ptr<BasePack>&& pack) = 0;
};

void initializeDispatchers();

Dispatcher* unstable();
Dispatcher* single();
Dispatcher* io();
Dispatcher* computation();

}
