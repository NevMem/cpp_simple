#include <cassert>
#include <fstream>
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

struct RunParams {
    double start;
    double finish;
    size_t iterationsCount;
    int countPoints;
};

int main() {
    auto mpi = mpi_adapter::createDefaultMpiAdapter();
    mpi->init();

    std::unique_ptr<RunParams> params;

    if (mpi->isMaster()) {
        double start, finish;
        double pointDelta, timeDelta;
        double time;
        
        {
            std::ifstream fin("input.txt");
            fin >> start >> finish;
            fin >> pointDelta >> timeDelta;
            fin >> time;
        }

        const size_t iterationsCount = time / timeDelta;
        const int countPoints = (finish - start) / pointDelta + 1;

        params = std::make_unique<RunParams>(RunParams { start, finish, iterationsCount, countPoints });

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

    auto solution = createDefaultSolution();
    auto result = solution->run(
        myTask,
        std::make_shared<DataAccessorImpl>(mpi, myTask.fromIndex, myTask.toIndex),
        std::make_shared<DataProviderImpl>(mpi, myTask.fromIndex, myTask.toIndex));

    if (!mpi->isMaster()) {
        for (const auto& elem : result) {
            mpi->send(
                ResultStreamEntity { elem.point, elem.result, elem.pointIndex },
                mpi->masterRank(),
                ResultStreamEntity::messageTag);        
        }
    } else {
        const auto remCount = params->countPoints - result.size();
        for (size_t i = 0; i != remCount; ++i) {
            auto entity = mpi->receiveFromAny<ResultStreamEntity>(ResultStreamEntity::messageTag);
            result.push_back(AtIndexResult { entity.index, entity.point, entity.value });
        }
        std::sort(result.begin(), result.end(), [](const auto& first, const auto& second) {
            return first.point < second.point;
        });
        for (const auto& elem : result) {
            std::cout << elem.point << " " << elem.result << std::endl;
        }
    }
}
