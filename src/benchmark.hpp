#pragma once
#include <chrono>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <vector>
#include "logger.hpp"
#include "ConsoleTM.hpp"
#include "AppMacros.hpp"
#include "obj.hpp"


namespace application{
    class benchmark{
    public:
        void start_clock();
        void end_clock();
        double speed(std::uintmax_t totalSize);
        void speed_test(const copyto& dir,std::uintmax_t bytes);
    private:
        std::chrono::steady_clock::time_point m_start,m_end;
        std::chrono::duration<double> m_duration;
    };
}