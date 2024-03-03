#pragma once
#include <chrono>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <vector>
#include "w32constants.hpp"
#include "w32sfct_api.hpp"


namespace application{
    class benchmark{
    public:
        // default constructor
        benchmark() = default;

        // default destructor
        ~benchmark()= default;
        
        // Copy constructor
        benchmark(const benchmark&) = delete;

        // Copy assignment operator
        benchmark& operator=(const benchmark&) = delete;

        // Move constructor
        benchmark(benchmark&&) = delete;

        // Move assignment operator
        benchmark& operator=(benchmark&&) = delete;   

        void start_clock() noexcept;
        void end_clock() noexcept;
        double_t speed(std::uintmax_t totalSize) noexcept;
        void speed_test(const copyto& dir,std::uintmax_t bytes) noexcept;
        void speed_test_4k(const copyto& dir,std::uintmax_t filesCount,std::uintmax_t bytes) noexcept;
        void speed_test_directories(const std::vector<copyto>& dirs) noexcept;
    private:
        std::chrono::steady_clock::time_point m_start,m_end;
        std::chrono::duration<double_t> m_duration;
    };
}