#pragma once

#include <qstring.h>
#include <unordered_set>

#include "cpu_data.h"
#include "../enums.h"

struct CpuCollector
{
    struct Options
    {
        const FilterMode filterMode;
        const std::unordered_set<QString> filter;
    };

    static Data_Cpu collect(const Options& options);
};