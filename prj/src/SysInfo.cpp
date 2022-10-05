
// clang-format off

#include "SysInfo.h"

#include <iostream>
#include <unistd.h>
#include <string.h>
#include <ctime>
#include <sys/time.h>
#include <linux/param.h>



// These Log definitions can be replaced with the LightLogger ones.
#define LLOG_DEBUG std::cout
#define LLOG_ERROR std::cerr



#define PRINT_MAP(v)                                                \
    for (const auto &val : (v))                                     \
        LOG_DEBUG << #v << "[" << val.first << "]:" << val.second;  \
    LOG_DEBUG << "";



bool ProcInfo::ReadValues(pid_t pid)
{
    _pid = pid;

    ReadProcStatValues();
    ReadProcStatusValues();
    ReadUpTimeValues();

    ReadMemInfoValues();

    ReadStatValues(_stats1);
    usleep(1000000);
    ReadStatValues(_stats2);

    //LOG_DEBUG << "ProcInfo::ReadValues:";
    //PRINT_MAP ( _procStats   );
    //PRINT_MAP ( _procStatsus );
    //PRINT_MAP ( _stats1      );
    //PRINT_MAP ( _stats2      );
    //PRINT_MAP ( _meminfo     );
    //PRINT_MAP ( _uptime      );

    return true;
}


bool ProcInfo::ReadStatValues(StatValues &vals)
{
    FILE *fp = fopen("/proc/stat", "r");
    if (nullptr == fp)
    {
        LLOG_ERROR << "Unable to read '/proc/stat'";
        return false;
    }

    char buffer[128];
    if (nullptr == fgets(buffer, 127, fp))
    {
        LLOG_ERROR << "Unable to read 'cpu' values";
        fclose(fp);
        return false;
    }
    fclose(fp);

    vals = StatValues{};

    char *bufferPrt = buffer;
    char *token     = nullptr;

    for (int idx = 0; nullptr != (token = strtok_r(bufferPrt, " \n", &bufferPrt)); ++idx)
    {
        uint64_t val = std::atoll(token);
        switch (idx)
        {
            case  1: vals["user"]        = val;  break;
            case  2: vals["nice"]        = val;  break;
            case  3: vals["system"]      = val;  break;
            case  4: vals["idle"]        = val;  break;
            case  5: vals["iowait"]      = val;  break;
            case  6: vals["irq"]         = val;  break;
            case  7: vals["softirq"]     = val;  break;
            case  8: vals["steal"]       = val;  break;
            case  9: vals["guest"]       = val;  break;
            case 10: vals["guestnice"]   = val;  break;
        }
    }

    //PRINT_MAP(vals);

    return true;
}

bool ProcInfo::ReadProcStatValues(ProcStatValues &vals) const
{
    std::string fname = "/proc/";
    fname += std::to_string(_pid) + "/stat";

    FILE *fp = fopen(fname.c_str(), "r");
    if (nullptr == fp)
    {
        LLOG_ERROR << "Unable to read " << fname;
        return false;
    }

    vals.clear();

    int  idx        = 0;
    char token[32]  = "";
    char *ptoken    = token;

    for (char ch = '\0'; ((ch = std::fgetc(fp)) != EOF);)
    {
        if (ch != ' ')
        {
            *ptoken++ = ch;
        }
        else
        {
            *ptoken = '\0';
            vals[idx++] = token;
            ptoken = token;
        }
    }
    fclose(fp);

    return true;
}

bool ProcInfo::ReadProcStatusValues(ProcStatusValues &vals) const
{
    std::string fname = "/proc/";
    fname += std::to_string(_pid) + "/status";

    FILE *fp = fopen(fname.c_str(), "r");
    if (nullptr == fp)
    {
        LLOG_ERROR << "Unable to read " << fname;
        return false;
    }

    vals.clear();

    char buffer[128]{};
    while (fgets(buffer, 127, fp))
    {
        char *bufferPrt = buffer;
        char *token     = nullptr;
        char *name      = nullptr;

        for (int idx = 0; nullptr != (token = strtok_r(bufferPrt, " :\n\t", &bufferPrt)); ++idx)
        {
            switch (idx)
            {
            case 0:
                name = token;
                break;
            case 1:
                if (name)
                    vals[name] = token;
            default:
                name = nullptr;
            }
        }
    }
    fclose(fp);

    return true;
}

bool ProcInfo::ReadMemInfoValues(MemInfoValues &vals) const
{
    FILE *fp = fopen("/proc/meminfo", "r");
    if (nullptr == fp)
    {
        LLOG_ERROR << "Unable to read '/proc/meminfo'";
        return false;
    }

    vals.clear();

    char buffer[128] {};
    while (fgets(buffer, 127, fp))
    {
        char *bufferPrt = buffer;
        char *token     = nullptr;
        char *name      = nullptr;

        for (int idx = 0; nullptr != (token = strtok_r(bufferPrt, " :\n", &bufferPrt)); ++idx)
        {
            switch (idx)
            {
            case 0:
                name = token;
                break;
            case 1:
                if (name)
                {
                    vals[name] = std::atoll(token);
                }
            default:
                name = nullptr;
            }
        }
    }
    fclose(fp);

    //PRINT_MAP(vals);

    return true;
}

bool ProcInfo::ReadUpTimeValues(UpTimeValues &vals) const
{
    FILE *fp = fopen("/proc/uptime", "r");
    if (nullptr == fp)
    {
        LLOG_ERROR << "Unable to read '/proc/uptime'";
        return false;
    }

    vals.clear();

    char buffer[128]{};
    while (fgets(buffer, 127, fp))
    {
        char *bufferPrt = buffer;
        char *token = nullptr;

        for (int idx = 0; nullptr != (token = strtok_r(bufferPrt, " :\n", &bufferPrt)); ++idx)
        {
            vals[idx] = token;
        }
    }
    fclose(fp);

    return true;
}

uint64_t ProcInfo::StatValuesSum(const StatValues &vals) const
{
    return vals.at("user")      +
           vals.at("nice")      +
           vals.at("system")    +
           vals.at("idle")      +
           vals.at("iowait")    +
           vals.at("irq")       +
           vals.at("softirq")   +
           vals.at("steal");
}

uint64_t ProcInfo::StatValuesIdle(const StatValues &vals) const
{
    return vals.at("idle")      +
           vals.at("iowait")    ;
}


double ProcInfo::SystemCpuUsage()
{
    uint64_t sum_diff  = StatValuesSum(_stats2)  - StatValuesSum(_stats1);
    uint64_t idle_diff = StatValuesIdle(_stats2) - StatValuesIdle(_stats1);
    if (0 != sum_diff)
        return 100.0 - ((double)idle_diff / (double)sum_diff) * 100.0;
    return 0.0;
}

double ProcInfo::SystemMemFree()
{
    return _meminfo["MemFree"];
}

double ProcInfo::SystemMemUsed()
{
    return _meminfo["MemTotal"] - _meminfo["MemFree"];
}

double ProcInfo::ProcessUpTime()
{
    std::time_t now = std::time(0);

    timeval tv;
    gettimeofday(&tv, 0);

    std::time_t boottime = tv.tv_sec - std::atoll(_uptime[0].c_str());
    return ((now - boottime) - (atof(_procStats[21].c_str())) / HZ);
}

double ProcInfo::ProcessThreadCount()
{
    return atoll(_procStats[19].c_str());
}

double ProcInfo::ProcessMemUsage()
{
    return atoll(_procStats[22].c_str()) / 1024.0;
}

double ProcInfo::ProcessMemoryUsed()
{
    return std::atoll(_procStatsus["VmRSS"].c_str()) / 1024.0;
}

// clang-format on

