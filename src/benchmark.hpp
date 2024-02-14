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
#include "windows_helper.hpp"
#include "constants.hpp"
#include "sfct_api.hpp"


namespace application{
    class benchmark{
    public:
        void start_clock();
        void end_clock();
        double_t speed(std::uintmax_t totalSize);
        void speed_test(const copyto& dir,std::uintmax_t bytes);
        void speed_test_4k(const copyto& dir,std::uintmax_t filesCount,std::uintmax_t bytes);
        void speed_test_directories(const std::vector<copyto>& dirs);
    private:
        std::chrono::steady_clock::time_point m_start,m_end;
        std::chrono::duration<double_t> m_duration;
    };
}