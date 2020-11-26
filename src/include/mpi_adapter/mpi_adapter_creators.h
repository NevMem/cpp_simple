#pragma once

#include <memory>

namespace mpi_adapter {

class MpiAdapter;

std::shared_ptr<MpiAdapter> createDefaultMpiAdapter();

}
