#pragma once

/**
 * In general should fetch value at any index
 * In this solution will be used to fetch data from other processors
 */
class DataAccessor {
public:
    ~DataAccessor() = default;
    
    virtual double getValueByIndex(int index) = 0;
};
