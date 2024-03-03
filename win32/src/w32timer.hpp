#pragma once
#include <chrono>
#include <cmath>
#include <thread>
#include <atomic>
#include <condition_variable>

////////////////////////////////////////////////
// This header contains a simple timer class. //                                           
////////////////////////////////////////////////

namespace application{
    /// @brief simple timer class
    class timer{
    public:
        /// @brief default destructor
        timer() = default;

        /// @brief default destructor
        ~timer() = default;
        
        /// @brief Copy constructor
        timer(const timer&) = delete;

        /// @brief Copy assignment operator
        timer& operator=(const timer&) = delete;

        /// @brief Move constructor
        timer(timer&&) = delete;

        /// @brief Move assignment operator
        timer& operator=(timer&&) = delete;

        /// @brief resets the members m_start, m_end and m_duration to zero
        void reset() noexcept;

        /// @brief begin the timer
        void start_clock() noexcept;

        /// @brief stop the timer
        void end_clock() noexcept;

        /// @brief returns the time elapsed in seconds using the time difference between m_start and m_end
        double_t get_time() noexcept;

        /// @brief causes a thread to sleep for seconds_to_wait
        /// @param seconds_to_wait number of seconds
        void wait_timer(double_t seconds_to_wait) noexcept;

        /// @brief used to notify another waiting thread after seconds_until_notify has timed out.
        /// @param seconds_until_notify number of seconds
        /// @param flag_notify notify flag that corresponds to notfiy_cv
        /// @param notify_cv the cv used to notify the outside thread
        /// @param start_timer the flag used to start the timer
        /// @param start_timer_cv the cv used to start the timer
        void notify_timer(double_t seconds_until_notify,std::atomic<bool>* flag_notify,std::condition_variable* notify_cv,std::atomic<bool>* start_timer,std::condition_variable* start_timer_cv) noexcept;
        
        /// @brief ends the notify timer sets m_running to false
        void end_notify_timer() noexcept;
    private:
        /// @brief the start and end times used in start_clock() and end_clock()
        std::chrono::steady_clock::time_point m_start,m_end;

        /// @brief the actual time elapsed
        std::chrono::duration<double_t> m_duration;

        /// @brief used to keep notify_timer running
        std::atomic<bool> m_running{true};
    };
}
