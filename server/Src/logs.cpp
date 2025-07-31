#include "logs.hpp"
#include <iostream>

Logfile::Logfile(const std::string& path) : path(path)
{
    std::ofstream ofs(path, std::ios::app);
    if (!ofs)
    {
        std::cerr << "[Logfile] Cannot open file: " << path << std::endl;
    }
}

void Logfile::threadsafe_log(const std::string& msg)
{
    std::lock_guard<std::mutex> lock(m);
    std::ofstream ofs(path, std::ios::app);
    if (ofs)
    {
        ofs << msg << std::endl;
    }
    else
    {
        std::cerr << "[Logfile] Failed to write to " << path << std::endl;
    }
}
