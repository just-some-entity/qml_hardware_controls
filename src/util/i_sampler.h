#pragma once
#include <qobject.h>

template<class In_t>
class IDataSampler : public QObject
{
public:
    ~IDataSampler() override = default;
    virtual void sample(In_t& data) = 0;
};