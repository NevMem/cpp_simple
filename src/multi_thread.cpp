#include "solution.h"

#include <unordered_map>
#include <mutex>

#include <threading/dispatcher/dispatchers.h>

namespace {

class Bitset {
public:
    explicit Bitset(size_t size)
    : size_(size)
    {
        const auto dataSize = (size_ + 63) / 64;
        data_.reserve(dataSize);
        for (size_t i = 0; i != dataSize; ++i) {
            data_.push_back(0);
        }
    }

    Bitset(const Bitset& other)
    : size_(other.size_)
    , data_(other.data_)
    {}

    Bitset(Bitset&& other)
    : size_(std::move(other.size_))
    , data_(std::move(other.data_))
    {}

    Bitset& operator=(const Bitset& other)
    {
        size_ = other.size_;
        data_ = other.data_;
        return *this;
    }

    Bitset& operator=(Bitset&& other)
    {
        size_ = other.size_;
        data_ = std::move(other.data_);
        return *this;
    }

    bool get(size_t index) const
    {
        const auto block = index / 64;
        return (data_[block] & (1ull << (block % 64))) != 0;
    }

    void set(size_t index, bool value)
    {
        const auto block = index / 64;
        if (value) {
            data_[block] |= (1ull << (block % 64));
        } else {
            data_[block] &= ~(1ull << (block % 64));
        }
    }

    std::vector<size_t> asVector() const
    {
        std::vector<size_t> res;
        for (size_t i = 0; i != size(); ++i) {
            if (get(i)) {
                res.push_back(i);
            }
        }
        return res;
    }

    size_t size() const
    {
        return size_;        
    }

private:
    size_t size_;
    std::vector<uint64_t> data_;
};

struct InternalResult {
    uint32_t cost;
    uint32_t capacity;
    std::set<size_t> included;
    std::set<size_t> excluded;
};
    
double calculateUpperBound(const std::vector<Item>& items, size_t capacity, const InternalResult& result)
{
    size_t remCap = capacity - result.capacity;
    double upperBound = result.cost;
    for (size_t i = 0; i != items.size(); ++i) {
        if (result.included.find(i) == result.included.end()
                && result.excluded.find(i) == result.excluded.end()) {
            size_t currentCap = std::min(remCap, static_cast<size_t>(items[i].size));
            upperBound += currentCap * (static_cast<double>(items[i].cost) / items[i].size);
            remCap -= currentCap;
        }
    }
    return upperBound;
}

class MultiThreadSolution : public Solution {
public:
    MultiThreadSolution()
    {}

    virtual Result solve(const std::vector<Item>& items, size_t capacity) override
    {
        std::vector<Item> sortedItems = items;
        std::vector<size_t> permutation;
        permutation.reserve(items.size());
        for (size_t i = 0; i != items.size(); ++i) {
            permutation.push_back(i);
        }
        std::sort(permutation.begin(), permutation.end(), [&items](size_t first, size_t second) {
            return items[first].cost * items[second].size > items[second].cost * items[first].size;
        });
        for (size_t i = 0; i != items.size(); ++i) {
            sortedItems[permutation[i]] = items[i];
        }
        run(InternalResult { 0, 0, {}, {} }, sortedItems, capacity);
        while (threading::dispatcher::computation()->hasTasks());
        return toResult(currentBest_, permutation);
    }

private:
    inline Result toResult(const InternalResult& result, const std::vector<size_t>& permutation) const
    {
        std::unordered_map<size_t, size_t> backPermutation;
        for (size_t i = 0; i != permutation.size(); ++i) {
            backPermutation[permutation[i]] = i;
        }
        std::set<unsigned int> res;
        for (const auto& value : result.included) {
            res.insert(backPermutation[value]);
        }
        return Result { result.cost, result.capacity, res };
    }

    void run(InternalResult current, const std::vector<Item>& items, size_t capacity)
    {
        {
            std::lock_guard<std::mutex> guard(bestResultMutex_);
            if (currentBest_.cost < current.cost) {
                currentBest_ = current;
            }
        }

        std::vector<std::future<int>> futures;
        for (size_t i = 0; i != items.size(); ++i) {
            if (current.capacity + items[i].size > capacity
                    || current.included.find(i) != current.included.end()
                    || current.excluded.find(i) != current.excluded.end()) {
                continue;
            }
            futures.push_back(threading::dispatcher::computation()->async([this, items, capacity, &current](size_t index) -> int {
                InternalResult copy = current;
                copy.included.insert(index);
                copy.cost += items[index].cost;
                copy.capacity += items[index].size;
                if (calculateUpperBound(
                        items,
                        capacity,
                        copy) >= currentBest_.cost) {
                    return static_cast<int>(index) + 1;
                } else {
                    return 0;
                }
            }, i));
            futures.push_back(threading::dispatcher::computation()->async([this, items, capacity, &current](size_t index) -> int {
                InternalResult copy = current;
                copy.excluded.insert(index);
                if (calculateUpperBound(
                        items,
                        capacity,
                        copy) >= currentBest_.cost) {
                    return -static_cast<int>(index) - 1;
                } else {
                    return 0;
                }
            }, i));
        }

        for (auto& future : futures) {
            const auto value = future.get();
            if (value == 0) {
                continue;
            }
            auto copy = current;
            if (value > 0) {
                copy.included.insert(value - 1);
                copy.capacity += items[value - 1].size;
                copy.cost += items[value - 1].cost;
                run(copy, items, capacity);
                continue;
            }
            copy.excluded.insert(-value - 1);
            run(copy, items, capacity);
        }
    }

    InternalResult currentBest_ = InternalResult { 0, 0, {}, {} };
    std::mutex bestResultMutex_;
};

}

std::unique_ptr<Solution> createMultiThreadSolution()
{
    return std::make_unique<MultiThreadSolution>();
}
