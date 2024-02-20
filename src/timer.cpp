#include "timer.hpp"

void application::timer::reset()
{
    // calling blank constructors to reset timer
    m_start = std::chrono::steady_clock::time_point();
    m_end = std::chrono::steady_clock::time_point();
    m_duration = std::chrono::duration<double_t>();
}

void application::timer::start_clock()
{
    m_start = std::chrono::high_resolution_clock::now();
}

void application::timer::end_clock()
{
    m_end = std::chrono::high_resolution_clock::now();
}

double_t application::timer::get_time() noexcept
{
    m_duration = m_end - m_start;
    return m_duration.count(); // in seconds
}

void application::timer::wait_timer(double_t seconds_to_wait) noexcept
{
    // Create a duration representing floating-point seconds.
    auto duration = std::chrono::duration<double_t>(seconds_to_wait);
    
    // sleep for seconds_to_wait
    std::this_thread::sleep_for(duration);
}

void application::timer::notify_timer(double_t seconds_until_notify, std::atomic<bool>* flag_notify, std::condition_variable* notify_cv,std::atomic<bool>* start_timer,std::condition_variable* start_timer_cv) noexcept
{
    while(m_running.load()){
        std::mutex local_mtx;
        std::unique_lock<std::mutex> local_lock(local_mtx);
        start_timer_cv->wait(local_lock, [start_timer] {return start_timer->load();});

        if(m_running.load()){
            wait_timer(seconds_until_notify);
            flag_notify->store(true);
            notify_cv->notify_one();
            start_timer->store(false);
        }
    }
}

void application::timer::end_notify_timer() noexcept
{
    m_running = false;
}
