#pragma once
#include <chrono>
#include <cmath>
#include <thread>
#include <atomic>
#include <condition_variable>

namespace application{
    class timer{
    public:
        void reset();
        void start_clock();
        void end_clock();
        double_t get_time();

        void wait_timer(double_t seconds_to_wait); 
        void notify_timer(double_t seconds_until_notify,std::atomic<bool>* flag_notify,std::condition_variable* notify_cv,std::atomic<bool>* start_timer,std::condition_variable* start_timer_cv);
        void end_notify_timer();
    private:
        std::chrono::steady_clock::time_point m_start,m_end;
        std::chrono::duration<double_t> m_duration;
        std::atomic<bool> m_running{true};
    };
}
