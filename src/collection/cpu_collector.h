#pragma once

#include <qstring.h>
#include <unordered_set>

#include "cpu_data.h"
#include "../enums.h"

class CpuCollector
{
public:
    struct Options
    {
        const FilterMode filterMode;
        const std::unordered_set<QString> filter;
    };

    Data_Cpu collect(const Options& options);
};