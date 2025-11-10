#pragma once

#include <QList>
#include <qstring.h>
#include <qtypes.h>

struct Data_Cpu
{
    struct Stats
    {
        quint64 user       = 0; // time spent in user mode
        quint64 nice       = 0; // time spent in user mode with low priority
        quint64 system     = 0; // time spent in kernel mode
        quint64 idle       = 0; // idle time
        quint64 iowait     = 0; // time waiting for I/O
        quint64 irq        = 0; // time servicing hardware interrupts
        quint64 softirq    = 0; // time servicing software interrupts
        quint64 steal      = 0; // time stolen by hypervisor
        quint64 guest      = 0; // running a virtual CPU
        quint64 guest_nice = 0; // guest time with low priority

        quint64 total() const
        {
            return nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
        }
    };

    struct EntryBase
    {
        float freqMin = 0.0;
        float freqMax = 0.0;
        float freqNow = 0.0;
        float temp    = 0.0;
    };

    struct Entry : EntryBase
    {
        Stats stats;
    };

    using CoreData = Entry;

    struct CpuData : Entry
    {
        QString name = nullptr;
        float   draw = 0.0;

        QVector<CoreData> cores;
    };

    float load1  = 0; // 1-minute load average
    float load5  = 0; // 5-minute load average
    float load15 = 0; // 15-minute load average

    QVector<CpuData> cpus;
};