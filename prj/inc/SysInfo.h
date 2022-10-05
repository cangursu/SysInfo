//Copyright 2019-2022 Afiniti, Inc
// clang-format off

#ifndef __PROC_INFO_H__
#define __PROC_INFO_H__


#include <stdint.h>
#include <string>
#include <map>

class ProcInfo
{
    public:
        using StatValues        = std::map<std::string, uint64_t>;
        using MemInfoValues     = std::map<std::string, uint64_t>;
        using ProcStatValues    = std::map<int, std::string>;
        using UpTimeValues      = std::map<int, std::string>;
        using ProcStatusValues  = std::map<std::string, std::string>;

        uint64_t StatValuesSum(const StatValues &vals) const;
        uint64_t StatValuesIdle(const StatValues &vals) const;

        bool    ReadValues(pid_t pid);
        bool    ReadStatValues(StatValues  &vals);

        bool    ReadProcStatValues(ProcStatValues &vals) const;
        bool    ReadProcStatusValues(ProcStatusValues &vals) const;
        bool    ReadMemInfoValues(MemInfoValues &vals) const;
        bool    ReadUpTimeValues(UpTimeValues &vals) const;

        bool    ReadProcStatValues()    {   return ReadProcStatValues(_procStats);      }
        bool    ReadProcStatusValues()  {   return ReadProcStatusValues(_procStatsus);  }
        bool    ReadMemInfoValues()     {   return ReadMemInfoValues(_meminfo);         }
        bool    ReadUpTimeValues()      {   return ReadUpTimeValues(_uptime);           }

        double  SystemCpuUsage();
        double  SystemMemFree();
        double  SystemMemUsed();
        double  ProcessUpTime();
        double  ProcessMemUsage();
        double  ProcessThreadCount();
        double  ProcessMemoryUsed();

    private:
        pid_t               _pid;
        StatValues          _stats1;
        StatValues          _stats2;
        MemInfoValues       _meminfo;
        ProcStatValues      _procStats;
        ProcStatusValues    _procStatsus;
        UpTimeValues        _uptime;
};




#endif // __PROC_INFO_H__
// clang-format on
