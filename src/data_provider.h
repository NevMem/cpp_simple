#pragma once

class DataProvider {
public:
    virtual ~DataProvider() = default;

    virtual void provideDataAt(int index, double value) = 0;
};
