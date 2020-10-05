#pragma once

#include <threading/dispatcher/dispatcher.h>

namespace threading::dispatcher {

Dispatcher* unstable();
Dispatcher* single();
Dispatcher* io();
Dispatcher* computation();

}
