#include <cstdio>
#include <algorithm>
#include <vector>
#include <cassert>

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

__global__ void saxpy(int n, float a, float *x, float *y)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) y[i] = a * x[i] + y[i];
}

int main(void)
{
    int N = 1 << 20;
    TypeArrayWrapper<float> x(N, 0), y(N, 0);

    for (int i = 0; i < N; i++) {
        x[i] = 1.0f;
        y[i] = 2.0f;
    }

    x.toDevice();
    y.toDevice();

    // Perform SAXPY on 1M elements
    saxpy<<<(N+255)/256, 256>>>(N, 2.0f, x.deviceData(), y.deviceData());

    y.toHost();

    float maxError = 0.0f;
    for (int i = 0; i < N; i++) {
        maxError = max(maxError, abs(y[i] - 4.0f));
    }
    printf("Max error: %f\n", maxError);
}