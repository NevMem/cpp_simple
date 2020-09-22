#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <set>
#include <future>

#include <threading/dispatcher/dispatchers.h>
#include <singleton/singleton.h>

class SomeClass {
public:
    std::string getMessage()
    {
        accesses_ += 1;
        return "Accesses: " + std::to_string(accesses_);
    }

private:
    int accesses_ = 0;
};

class MyException : public std::exception {
public:
    virtual const char* what() const noexcept override
    {
        return "My exception";
    }
};

int main() {
    threading::dispatcher::initializeDispatchers();

    const size_t num_threads = 1000;
    size_t globSum = 0;
    std::mutex mutex;

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::future<size_t>> futures;

    size_t check = 0;

    for (size_t i = 0; i != num_threads; ++i) {
        auto future = threading::dispatcher::computation()->async([](size_t a, size_t& kek) -> size_t {
            size_t sum = 0;
            if (a == 15) {
                throw MyException();
            }
            for (size_t i = a * 100000; i != (a + 1) * 100000; ++i) {
                sum += i;
            }
            kek += 1;
            return sum;
        }, i, std::ref(check));
        futures.push_back(std::move(future));
    }

    std::cout << check << std::endl;
    
    for (auto& future : futures) {
        try {
            globSum += future.get();
        } catch (const MyException& exception) {
            std::cout << "exception what: " << exception.what() << std::endl;
        }
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::cout << globSum << std::endl;
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count() << std::endl;
}
