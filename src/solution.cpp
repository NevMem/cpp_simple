#include "solution.h"

#include <unordered_map>

namespace {

constexpr double INITIAL_VALUE = 1.0;

class CompositeDataAccessor final : DataAccessor {
public:
    CompositeDataAccessor(const std::shared_ptr<DataAccessor>& dataAccessor, const std::shared_ptr<DataProvider>& dataProvider)
    : externalDataAccessor_(dataAccessor)
    , dataProvider_(dataProvider)
    {}

    virtual double getValueByIndex(int index) override
    {
        for (size_t i = 0; i != internalResults_.size(); ++i) {
            if (internalResults_[i].pointIndex == index) {
                return internalResults_[i].result;
            }
        }
        return externalDataAccessor_->getValueByIndex(index);
    }

    void updateInternalResults(const std::vector<AtIndexResult>& internalResults)
    {
        internalResults_ = internalResults;
        dataProvider_->provideDataAt(internalResults_.front().pointIndex, getValueByIndex(internalResults_.front().pointIndex));
        dataProvider_->provideDataAt(internalResults_.back().pointIndex, getValueByIndex(internalResults_.back().pointIndex));   
    }

private:
    const std::shared_ptr<DataAccessor> externalDataAccessor_;
    const std::shared_ptr<DataProvider> dataProvider_;

    std::vector<AtIndexResult> internalResults_;
};

class DefaultSolution final : public Solution {
public:
    virtual std::vector<AtIndexResult> run(
        const Task& task,
        const std::shared_ptr<DataAccessor>& dataAccessor,
        const std::shared_ptr<DataProvider>& dataProvider) const override
    {
        std::vector<AtIndexResult> result;
        const size_t totalPoints = task.toIndex - task.fromIndex + 1;
        result.reserve(totalPoints);
        for (size_t i = 0; i != totalPoints; ++i) {
            const double point = task.fromPoint + (task.toPoint - task.fromPoint) * i / totalPoints;
            result.push_back(AtIndexResult { task.fromIndex + i, point, INITIAL_VALUE });
        }

        const auto compositeDataAccessor = std::make_shared<CompositeDataAccessor>(dataAccessor, dataProvider);
        compositeDataAccessor->updateInternalResults(result);

        std::vector<int> order;
        order.reserve(result.size());
        for (size_t i = 1; i != result.size(); ++i) {
            order.push_back(i);
        }
        order.push_back(0);

        for (size_t it = 0; it != task.iterationsCount; ++it) {
            for (const auto& index : order) {
                const double inBeforePoint = compositeDataAccessor->getValueByIndex(result[index].pointIndex - 1);
                const double inPoint = compositeDataAccessor->getValueByIndex(result[index].pointIndex);
                const double inNextPoint = compositeDataAccessor->getValueByIndex(result[index].pointIndex + 1);
                result[index].result = inPoint + task.coef * task.deltaTime * (inBeforePoint + inNextPoint - 2 * inPoint) / task.deltaSpace / task.deltaSpace;
            }
            compositeDataAccessor->updateInternalResults(result);  
        }

        return result;
    }

};

}

std::shared_ptr<Solution> createDefaultSolution()
{
    return std::make_shared<DefaultSolution>();
}

