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

class MultiWaitFuture {
public:
    MultiWaitFuture(std::vector<std::future<std::future<void>>>&& futures)
    : futures_(std::forward<std::vector<std::future<std::future<void>>>>(futures))
    {}

    void wait()
    {
        for (auto& future: futures_) {
            future.get().get();
        }
    }

private:
    std::vector<std::future<std::future<void>>> futures_;
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
        run(InternalResult { 0, 0, Bitset(items.size()) }, sortedItems, capacity);
        while (threading::dispatcher::computation()->hasTasks());
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

    void run(InternalResult current, const std::vector<Item>& items, size_t capacity, size_t minUnusedIndex = 0)
    {
        {
            std::lock_guard<std::mutex> guard(bestResultMutex_);
            if (currentBest_.cost < current.cost) {
                std::cout << "Update best result" << std::endl;
                currentBest_ = current;
            }
        }

        for (size_t i = minUnusedIndex; i != items.size(); ++i) {
            if (current.capacity + items[i].size > capacity) {
                continue;
            }
            if (calculateUpperBound(
                    items,
                    capacity,
                    current.capacity + items[i].size,
                    current.cost + items[i].cost,
                    i + 1) <= currentBest_.cost) {
                continue;
            }
            const auto index = i;

            auto newResult = InternalResult { current.cost + items[index].cost, current.capacity + items[index].size, current.used };
            newResult.used.set(index, true);

            // threading::dispatcher::computation()->async([this](
            //         InternalResult&& newResult, const std::vector<Item>& items, size_t capacity, size_t index) {
                run(std::forward<InternalResult>(newResult), items, capacity, index + 1);
            // }, std::move(newResult), std::cref(items), capacity, index);
        }
    }

    InternalResult currentBest_ = InternalResult { 0, 0, Bitset(0) };
    std::mutex bestResultMutex_;
};

}

std::unique_ptr<Solution> createMultiThreadSolution()
{
    return std::make_unique<MultiThreadSolution>();
}
