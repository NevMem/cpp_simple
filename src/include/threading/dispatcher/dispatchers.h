#pragma once

#include <threading/dispatcher/dispatcher.h>

namespace threading::dispatcher {

void initializeDispatchers();

Dispatcher* unstable();
Dispatcher* single();
Dispatcher* io();
Dispatcher* computation();

}
