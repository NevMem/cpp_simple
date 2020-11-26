#include <cassert>
#include <functional>
#include <future>
#include <iostream>
#include <optional>
#include <set>
#include <thread>
#include <vector>

#include <mpi_adapter/mpi_adapter.h>
#include <mpi_adapter/mpi_adapter_creators.h>

#include "message.h"
#include "solution.h"

class DataAccessorImpl final : public DataAccessor {
public:
    DataAccessorImpl(const std::shared_ptr<mpi_adapter::MpiAdapter>& adapter, int myFromIndex, int myToIndex)
    : adapter_(adapter)
    , myFromIndex_(myFromIndex)
    , myToIndex_(myToIndex)
    {}

    virtual double getValueByIndex(int index) override
    {
        assert(index < myFromIndex_ || myToIndex_ < index);
        if (index == -1) {
            return 0;
        }
        if (index > myToIndex_ && adapter_->worldSize() == adapter_->rank() + 1) {
            return 0;
        }
        if (index < myFromIndex_) {
            return adapter_->receive<Value>(adapter_->rank() - 1, Value::messageTag).value;
        }
        return adapter_->receive<Value>(adapter_->rank() + 1, Value::messageTag).value;
    }

private:
    const std::shared_ptr<mpi_adapter::MpiAdapter> adapter_;
    const int myFromIndex_;
    const int myToIndex_;
};

class DataProviderImpl final : public DataProvider {
public:
    DataProviderImpl(const std::shared_ptr<mpi_adapter::MpiAdapter>& adapter, int myFromIndex, int myToIndex)
    : adapter_(adapter)
    , myFromIndex_(myFromIndex)
    , myToIndex_(myToIndex)
    {}

    virtual void provideDataAt(int index, double value) override
    {
        assert(index == myFromIndex_ || myToIndex_ == index);
        if (index == 0) {
            return;
        }
        if (index == myToIndex_ && adapter_->worldSize() == adapter_->rank() + 1) {
            return;
        }
        if (index == myFromIndex_) {
            return adapter_->send<Value>(Value(value), adapter_->rank() - 1, Value::messageTag);
        }
        return adapter_->send<Value>(Value(value), adapter_->rank() + 1, Value::messageTag);
    }

private:
    const std::shared_ptr<mpi_adapter::MpiAdapter> adapter_;
    const int myFromIndex_;
    const int myToIndex_;
};

int main() {
    auto mpi = mpi_adapter::createDefaultMpiAdapter();
    mpi->init();

    if (mpi->isMaster()) {
        const double start = 0;
        const double finish = 1;
        const double pointDelta = 0.02;
        const double timeDelta = 0.0002;
        const size_t iterationsCount = 0.1 / timeDelta;

        const int countPoints = (finish - start) / pointDelta + 1;

        const int executors = mpi->worldSize();
        std::vector<Task> tasks;
        tasks.reserve(executors);

        for (size_t i = 0; i != executors; ++i) {
            const int fromPoint = countPoints / executors * i;
            const int toPoint = i == executors - 1
                ? countPoints - 1
                : fromPoint + countPoints / executors - 1;
            const double from = start + pointDelta * fromPoint;
            const double to = start + pointDelta * toPoint;
            tasks.push_back(Task { from, to, fromPoint, toPoint, timeDelta, pointDelta, iterationsCount, 1.0 });
        }

        for (size_t i = 0; i != tasks.size(); ++i) {
            mpi->send(tasks[i], i, Task::messageTag);
        }
    }

    Task myTask = mpi->receive<Task>(0, Task::messageTag);

    std::cout << myTask.fromIndex << " " << myTask.toIndex << " " << myTask.fromPoint << " " << myTask.toPoint << " " << myTask.iterationsCount << std::endl;

    auto solution = createDefaultSolution();
    auto result = solution->run(
        myTask,
        std::make_shared<DataAccessorImpl>(mpi, myTask.fromIndex, myTask.toIndex),
        std::make_shared<DataProviderImpl>(mpi, myTask.fromIndex, myTask.toIndex));
    for (const auto& elem : result) {
        std::cout << elem.pointIndex << " " << elem.point << " " << elem.result << std::endl;
    }
}
