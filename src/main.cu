#include <cstdio>
#include <algorithm>
#include <vector>
#include <cassert>
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>

template <typename T>
class TypeArrayWrapper {
public:
    TypeArrayWrapper(const std::vector<T>& data)
    {
        data_ = new T[data.size()];
        size_ = data.size();
        for (size_t i = 0; i != size_; ++i) {
            data_[i] = data[i];
        }
    }

    TypeArrayWrapper(size_t size, T fill)
    {
        size_ = size;
        data_ = new T[size];
        for (size_t i = 0; i != size; ++i) {
            data_[i] = fill;
        }
    }

    bool isOnDevice() const
    {
        return onDevice_;
    }

    T* deviceData() const
    {
        assert(onDevice_);
        return deviceData_;
    }

    void toDevice()
    {
        assert(!onDevice_);
        initDeviceDataIfNeeded();
        cudaMemcpy(deviceData_, data_, size_ * sizeof(T), cudaMemcpyHostToDevice);
        onDevice_ = true;
    }

    void toHost()
    {
        assert(onDevice_);
        assert(deviceData_);
        cudaMemcpy(data_, deviceData_, size_ * sizeof(T), cudaMemcpyDeviceToHost);
        onDevice_ = false;
    }

    size_t size() const
    {
        return size_;
    }

    T& operator[](size_t index)
    {
        assert(!onDevice_);
        return data_[index];
    }

    std::vector<T> asVector()
    {
        assert(!onDevice_);
        std::vector<T> result;
        result.resize(size_);
        for (size_t i = 0; i != size_; ++i) {
            result[i] = data_[i];
        }
        return result;
    }

    void clear()
    {
        clearAll();
    }

    ~TypeArrayWrapper()
    {
        clearAll();
    }

private:
    void clearAll()
    {
        if (data_) {
            free(data_);
            data_ = nullptr;
        }
        if (deviceData_) {
            cudaFree(deviceData_);
            deviceData_ = nullptr;
        }
    }

    void initDeviceDataIfNeeded()
    {
        if (!deviceData_) {
            cudaMalloc(&deviceData_, size_ * sizeof(T));
        }
    }

    T* data_ = nullptr;
    T* deviceData_ = nullptr;
    size_t size_ = 0;

    bool onDevice_ = false;
};

std::vector<int> addPadding(std::vector<int> pixels, size_t height, size_t width, size_t padding)
{
    const auto coordPrev = [&height, &width](const size_t i, const size_t j) -> size_t {
        return i * width + j;
    };
    const auto coordNew = [&height, &width, &padding](const size_t i, const size_t j) -> size_t {
        return i * (width + padding * 2) + j;
    };

    std::vector<int> result;
    result.resize((height + padding * 2) * (width + padding * 2));
    for (size_t i = 0; i != height + padding * 2; ++i) {
        for (size_t j = 0; j != width + padding * 2; ++j) {
            if (padding <= i && padding <= j && i < height + padding && j < width + padding) {
                result[coordNew(i, j)] = pixels[coordPrev(i - padding, j - padding)];
            } else if (i < padding) {
                size_t coordJ = j < padding
                    ? 0
                    : (j >= width + padding ? width - 1 : j - padding);
                result[coordNew(i, j)] = pixels[coordPrev(0, coordJ)];
            } else if (i >= height + padding) {
                size_t coordJ = j < padding
                    ? 0
                    : (j >= width + padding ? width - 1 : j - padding);
                result[coordNew(i, j)] = pixels[coordPrev(height - 1, coordJ)];
            } else {
                result[coordNew(i, j)] = pixels[coordPrev(i - padding, j < padding ? 0 : width - 1)];
            }
        }
    }
    return result;
}

std::vector<int> removePadding(std::vector<int> pixels, size_t height, size_t width, size_t padding)
{
    const auto coordPrev = [&height, &width](const size_t i, const size_t j) -> size_t {
        return i * width + j;
    };
    const auto coordNew = [&height, &width, &padding](const size_t i, const size_t j) -> size_t {
        return i * (width - padding * 2) + j;
    };

    std::vector<int> result;
    result.resize((height - padding * 2) * (width - padding * 2));
    for (size_t i = 0; i != height; ++i) {
        for (size_t j = 0; j != width; ++j) {
            if (padding <= i && padding <= j && i < height - padding && j < width - padding) {
                result[coordNew(i - padding, j - padding)] = pixels[coordPrev(i, j)];
            }
        }
    }
    return result;
}

std::vector<float> createTransformMatrix(size_t size)
{
    assert(size % 2 == 1);
    int p = size / 2;
    float sigma = 1;
    std::vector<float> result;
    result.reserve(size * size);
    for (int x = -p; x <= p; ++x) {
        for (int y = -p; y <= p; ++y) {
            result.push_back(exp(-(x * x + y * y) / (2 * sigma * sigma)));
        }
    }
    float sum = 0;
    for (const auto& elem : result) {
        sum += elem;
    }
    for (auto& elem : result) {
        elem /= sum;
    }
    return result;
}

__global__ void transform(
    const int height,
    const int width,
    const int maskSize,
    const int* const from,
    int* const to,
    const float* const mask)
{
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    const int i = index / width;
    const int j = index - i * width;
    if (i < height && j < width) {
        const int pad = maskSize / 2;
        if (pad <= i && pad <= j && i < height - pad && j < width - pad) {
            float sum = 0;
            for (int dx = -pad; dx <= pad; ++dx) {
                for (int dy = -pad; dy <= pad; ++dy) {
                    const int maskIndex = (dx + pad) * maskSize + dy + pad;
                    const int fromIndex = (i + dx) * width + (j + dy);
                    sum += mask[maskIndex] * from[fromIndex];
                }
            }
            to[i * width + j] = sum;
        } else {
            to[i * width + j] = from[i * width + j];
        }
    }
}

std::vector<int> applyTransform(std::vector<int> pixels, size_t height, size_t width, size_t maskSize)
{
    const auto matrix = createTransformMatrix(maskSize);

    TypeArrayWrapper matr(matrix);
    matr.toDevice();

    TypeArrayWrapper data(pixels);
    data.toDevice();

    TypeArrayWrapper<int> result(pixels.size(), 0);
    result.toDevice();

    transform<<<(width * height + 511) / 512, 512>>>(height, width, maskSize, data.deviceData(), result.deviceData(), matr.deviceData());

    result.toHost();
    return result.asVector();
}


std::vector<int> applyTrnasformOnCPU(std::vector<int> pixels, size_t height, size_t width, size_t maskSize)
{
    const auto matrix = createTransformMatrix(maskSize);
    std::vector<int> result;
    result.resize(pixels.size(), 0);

    for (size_t index = 0; index != result.size(); ++index) {
        const int i = index / width;
        const int j = index - i * width;
        if (i < height && j < width) {
            const int pad = maskSize / 2;
            if (pad <= i && pad <= j && i < height - pad && j < width - pad) {
                float sum = 0;
                for (int dx = -pad; dx <= pad; ++dx) {
                    for (int dy = -pad; dy <= pad; ++dy) {
                        const int maskIndex = (dx + pad) * maskSize + dy + pad;
                        const int fromIndex = (i + dx) * width + (j + dy);
                        sum += matrix[maskIndex] * pixels[fromIndex];
                    }
                }
                result[i * width + j] = sum;
            } else {
                result[i * width + j] = pixels[i * width + j];
            }
        }
    }

    return result;
}

class Timer {
public:
    Timer(const std::string& tag)
    : tag_(tag)
    , start_(std::chrono::high_resolution_clock::now())
    {}

    ~Timer()
    {
        const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_).count();
        std::cerr << "[" << tag_ << "] " << diff << std::endl;
    }

private:
    const std::string tag_;
    const std::chrono::steady_clock::time_point start_;
};

int main(int argc, char** argv)
{
    assert(argc > 3);
    
    const std::string filename = argv[1];
    std::ifstream fin(filename);

    const std::string mode = argv[2];
    assert(mode == "cuda" || mode == "cpu");

    const std::string kernelSize = argv[3];
    const size_t kernel = atoi(kernelSize.c_str());
    assert(kernel % 2 == 1);
    const size_t padding = kernel / 2;

    size_t height, width;
    fin >> height >> width;

    std::cout << height << ' ' << width << '\n';

    for (const auto& channel : {0, 1, 2}) {
        std::vector<int> pixels;
        {
            Timer timer("Reading channel " + std::to_string(channel));
            for (size_t i = 0; i != height; ++i) {
                for (size_t j = 0; j != width; ++j) {
                    int value;
                    fin >> value;
                    pixels.push_back(value);
                }
            }
        }
        {
            Timer timer("Adding padding");
            pixels = addPadding(std::move(pixels), height, width, padding);
        }
        for (int i = 0; i != 10; ++i) {
            Timer timer("Transforming");
            if (mode == "cuda") {
                pixels = applyTransform(
                    std::move(pixels),
                    height + padding * 2,
                    width + padding * 2,
                    padding * 2 + 1);
            } else {
                pixels = applyTrnasformOnCPU(
                    std::move(pixels),
                    height + padding * 2,
                    width + padding * 2,
                    padding * 2 + 1);
            }
        }
        {
            Timer timer("Removing padding");
            pixels = removePadding(std::move(pixels), height + padding * 2, width + padding * 2, padding);
        }

        {
            Timer("Writing channel " + std::to_string(channel));
            for (size_t i = 0; i != height; ++i) {
                for (size_t j = 0; j != width; ++j) {
                    std::cout << pixels[i * width + j] << ' ';
                }
                std::cout << '\n';
            }
        }
    }
}
