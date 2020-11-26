#pragma once

namespace mpi_adapter {

class MpiAdapter {
public:
    virtual ~MpiAdapter() = default;

    virtual bool isMaster() = 0;
    virtual int rank() = 0;

    virtual int worldSize() = 0;

    template<typename T>
    void send(const T& value, int dest, int tag = 0)
    {
        doSend(&value, sizeof(T), dest, tag);
    }

    template<typename T>
    T receive(int src, int tag = 0)
    {
        auto received = doReceive(sizeof(T), src, tag);
        T result = *reinterpret_cast<const T*>(received.data);
        delete[] received.data;
        return result;
    }

    virtual void init() = 0;
    virtual void finalize() = 0;

protected:
    struct ReceivedData {
        const char* data;
    };

    virtual void doSend(const void* data, int size, int dest, int tag) = 0;
    virtual ReceivedData doReceive(int size, int src, int tag) = 0;
};

}

