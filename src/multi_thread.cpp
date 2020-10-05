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
        return (data_[block] & (1ull << (index % 64))) != 0;
    }

    void set(size_t index, bool value)
    {
        const auto block = index / 64;
        if (value) {
            data_[block] |= (1ull << (index % 64));
        } else {
            data_[block] &= ~(1ull << (index % 64));
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
        if (result.included.find(i) == result.included.end() && result.excluded.find(i) == result.excluded.end()) {
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
        run(sortedItems, capacity);
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

    void run(const std::vector<Item>& items, size_t capacity)
    {
        {
            if (currentBest_.cost < current_.cost) {
                currentBest_ = current_;
            }
        }

        std::vector<std::future<void>> futures;
        std::vector<int> values;
        std::mutex writeMutex;
        for (size_t i = 0; i != items.size(); ++i) {
            if (current_.included.find(i) != current_.included.end() || current_.excluded.find(i) != current_.excluded.end()) {
                continue;
            }
            
            futures.push_back(threading::dispatcher::computation()->async([this, &items, &writeMutex, capacity](size_t index, std::vector<int>& values) {
                if (current_.capacity + items[index].size <= capacity) {
                    InternalResult copy = current_;
                    copy.included.insert(index);
                    copy.cost += items[index].cost;
                    copy.capacity += items[index].size;
                    if (calculateUpperBound(
                            items,
                            capacity,
                            copy) >= currentBest_.cost) {
                        std::lock_guard<std::mutex> guard(writeMutex);
                        values.push_back(static_cast<int>(index) + 1);
                    }
                }
            }, i, std::ref(values)));
            futures.push_back(threading::dispatcher::computation()->async([this, &items, &writeMutex, capacity](size_t index, std::vector<int>& values) {
                InternalResult copy = current_;
                copy.excluded.insert(index);
                if (calculateUpperBound(
                        items,
                        capacity,
                        copy) >= currentBest_.cost) {
                    std::lock_guard<std::mutex> guard(writeMutex);
                    values.push_back(-static_cast<int>(index) - 1);
                }
            }, i, std::ref(values)));
        }

        for (auto& future : futures) {
            future.get();
        }

        for (const auto& value : values) {
            if (value > 0) {
                current_.included.insert(value - 1);
                current_.capacity += items[value - 1].size;
                current_.cost += items[value - 1].cost;
                run(items, capacity);
                current_.included.erase(value - 1);
                current_.capacity -= items[value - 1].size;
                current_.cost -= items[value - 1].cost;
                continue;
            }
            current_.excluded.insert(-value - 1);
            run(items, capacity);
            current_.excluded.erase(-value - 1);
        }
    }

    InternalResult currentBest_ = InternalResult { 0, 0, {}, {} };
    InternalResult current_ = InternalResult { 0, 0, {}, {} };
};

}

std::unique_ptr<Solution> createMultiThreadSolution()
{
    return std::make_unique<MultiThreadSolution>();
}
