#include "solution.h"

#include <unordered_map>

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

double calculateUpperBound(
    const std::vector<Item>& items,
    size_t capacity,
    size_t currentCapactity,
    size_t currentCost,
    size_t minUnusedIndex)
{
    double upperBound = currentCost;
    size_t remainingCap = capacity - currentCapactity;
    for (size_t i = minUnusedIndex; i != items.size() && remainingCap != 0; ++i) {
        size_t capNow = std::min(static_cast<uint32_t>(remainingCap), items[i].size);
        upperBound += capNow * (static_cast<double>(items[i].cost) / items[i].size);
        remainingCap -= capNow;
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
        current_.used = Bitset(items.size());
        run(sortedItems, capacity);
        return toResult(currentBest_, permutation);
    }

private:
    struct InternalResult {
        uint32_t cost;
        uint32_t capacity;
        Bitset used;
    };

    inline Result toResult(const InternalResult& result, const std::vector<size_t>& permutation) const
    {
        std::unordered_map<size_t, size_t> backPermutation;
        for (size_t i = 0; i != permutation.size(); ++i) {
            backPermutation[i] = permutation[i];
        }
        std::set<unsigned int> res;
        for (const auto& value : result.used.asVector()) {
            res.insert(backPermutation[value]);
        }
        return Result { result.cost, result.capacity, res };
    }

    void run(const std::vector<Item>& items, size_t capacity, size_t minUnusedIndex = 0)
    {
        if (currentBest_.cost < current_.cost) {
            currentBest_ = current_;
        }

        std::vector<std::future<size_t>> validation;

        for (size_t i = minUnusedIndex; i != items.size(); ++i) {
            if (current_.capacity + items[i].size <= capacity) {
                validation.push_back(threading::dispatcher::computation()->async([this, capacity](size_t i, const std::vector<Item>& items) -> size_t {
                    if (calculateUpperBound(
                        items,
                        capacity,
                        current_.capacity + items[i].size,
                        current_.cost + items[i].cost,
                        i + 1) >= currentBest_.cost) {
                        return i;
                    } else {
                        return -1;
                    }
                }, i, std::cref(items)));
            }
        }

        for (auto& future : validation) {
            const auto index = future.get();
            if (index == -1) continue;
            current_.capacity += items[index].size;
            current_.cost += items[index].cost;
            current_.used.set(index, true);

            run(items, capacity, index + 1);

            current_.used.set(index, false);
            current_.capacity -= items[index].size;
            current_.cost -= items[index].cost;
        }
    }

    InternalResult currentBest_ = InternalResult { 0, 0, Bitset(0) };
    InternalResult current_ = InternalResult { 0, 0, Bitset(0) };
};

}

std::unique_ptr<Solution> createMultiThreadSolution()
{
    return std::make_unique<MultiThreadSolution>();
}
