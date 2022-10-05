
#include "SysInfo.h"

#include <iostream>
#include <unistd.h>

int main(int argc, const char * argv[])
{
    std::cout << "Hello\n";

    sleep(1);

    ProcInfo pinfo;
    pinfo.ReadValues(getpid());

    std::cout << "SystemCpuUsage     : "    << std::dec << pinfo.SystemCpuUsage()       << "\n";
    std::cout << "SystemMemFree      : "    << std::dec << pinfo.SystemMemFree()        << "\n";
    std::cout << "SystemMemUsed      : "    << std::dec << pinfo.SystemMemUsed()        << "\n";
    std::cout << "ProcessUpTime      : "    << std::dec << pinfo.ProcessUpTime()        << "\n";
    std::cout << "ProcessMemUsage    : "    << std::dec << pinfo.ProcessMemUsage()      << "\n";
    std::cout << "ProcessThreadCount : "    << std::dec << pinfo.ProcessThreadCount()   << "\n";
    std::cout << "ProcessMemoryUsed  : "    << std::dec << pinfo.ProcessMemoryUsed()    << "\n";

    return 0;
}
