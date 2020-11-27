#include <mpi_adapter/mpi_adapter.h>
#include <mpi_adapter/mpi_adapter_creators.h>

#include <cassert>
#include <optional>

#include <mpi.h>

namespace mpi_adapter {

namespace {

constexpr int MASTER_RANK = 0;

class DefaultMpiAdapter final : public MpiAdapter {
public:
    virtual ~DefaultMpiAdapter()
    {
        if (!isFinalized_) {
            finalize();
        }
    }

    DefaultMpiAdapter()
    {}

    virtual bool isMaster() override
    {
        const static bool isMaster = rank() == MASTER_RANK;
        return isMaster;
    }

    virtual int masterRank() const override
    {
        return MASTER_RANK;
    }

    virtual int rank() override
    {
        if (!rank_) {
            rank_ = -1;
            assert(MPI_Comm_rank(MPI_COMM_WORLD, &*rank_) == MPI_SUCCESS);
        }
        return *rank_;
    }

    virtual int worldSize() override
    {
        int worldSize = 0;
        assert(MPI_Comm_size(MPI_COMM_WORLD, &worldSize) == MPI_SUCCESS);
        return worldSize;
    }

    virtual void init() override
    {
        assert(!isInialized_);
        MPI_Init(NULL, NULL);
        isInialized_ = true;
    }

    virtual void finalize() override
    {
        assert(!isFinalized_);
        MPI_Finalize();
        isFinalized_  = true;
    }

protected:
    virtual void doSend(const void* data, int size, int dest, int tag) override
    {
        MPI_Send(data, size, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
    }

    virtual ReceivedData doReceive(int size, int src, int tag) override
    {
        char* buffer = new char[size];
        MPI_Recv(buffer, size, MPI_CHAR, src, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        return ReceivedData { buffer };
    }

private:
    bool isInialized_ = false;
    bool isFinalized_ = false;

    std::optional<int> rank_;
};

}

std::shared_ptr<MpiAdapter> createDefaultMpiAdapter()
{
    return std::make_shared<DefaultMpiAdapter>();
}

}
