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

double_t application::timer::get_time()
{
    m_duration = m_end - m_start;
    return m_duration.count(); // in seconds
}

void application::timer::wait_timer(double_t seconds_to_wait)
{
    // Create a duration representing floating-point seconds.
    auto duration = std::chrono::duration<double_t>(seconds_to_wait);
    
    // sleep for seconds_to_wait
    std::this_thread::sleep_for(duration);
}

void application::timer::notify_timer(double_t seconds_until_notify, std::atomic<bool>* flag_notify, std::condition_variable* cv,std::atomic<bool>* times_up)
{
    wait_timer(seconds_until_notify);
    flag_notify->store(true);
    cv->notify_one();
    times_up->store(false);
}
