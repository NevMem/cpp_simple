#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <set>
#include <future>

std::shared_ptr< std::vector<int> > smth()
{
    return std::make_shared<std::vector<int> >(
        std::initializer_list<int> { 1, 2, 3, 4 }
    );
}

int main() {
    std::cout << "here" << std::endl;
    const auto vec = *smth();
    for (const int& elem: vec) {
        std::cout << "i'm here" << std::endl;
        std::cout << elem << std::endl;
    }
    std::cout << "after" << std::endl;
}
