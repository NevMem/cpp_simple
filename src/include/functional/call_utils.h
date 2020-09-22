#pragma once

#include <cstddef>

namespace functional {

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
