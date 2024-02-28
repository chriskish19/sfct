#pragma once
#include <chrono>
#include <cmath>
#include <thread>
#include <atomic>
#include <condition_variable>

namespace application{
    class timer{
    public:
        void reset() noexcept;
        void start_clock() noexcept;
        void end_clock() noexcept;
        double_t get_time() noexcept;

        void wait_timer(double_t seconds_to_wait) noexcept; 
        void notify_timer(double_t seconds_until_notify,std::atomic<bool>* flag_notify,std::condition_variable* notify_cv,std::atomic<bool>* start_timer,std::condition_variable* start_timer_cv) noexcept;
        void end_notify_timer() noexcept;
    private:
        std::chrono::steady_clock::time_point m_start,m_end;
        std::chrono::duration<double_t> m_duration;
        std::atomic<bool> m_running{true};
    };
}
