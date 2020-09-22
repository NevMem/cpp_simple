#pragma once

#include <memory>

namespace singleton {

template<typename T, size_t identifier = 0>
T& singleton()
{
    static T* instance = new T;
    return *instance;
}
    
}
