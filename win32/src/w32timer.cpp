#include "w32timer.hpp"

void application::timer::reset() noexcept
{
    // calling blank constructors to reset timer
    m_start = std::chrono::steady_clock::time_point();
    m_end = std::chrono::steady_clock::time_point();
    m_duration = std::chrono::duration<double_t>();
}

void application::timer::start_clock() noexcept
{
    m_start = std::chrono::high_resolution_clock::now();
}

void application::timer::end_clock() noexcept
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
        
        // we attempt to cause the thread to wait
        // if that throws we exit the timer loop
        try{
            std::unique_lock<std::mutex> local_lock(local_mtx);
            start_timer_cv->wait(local_lock, [start_timer] {return start_timer->load();});
        }
        catch(const std::runtime_error& e){
            std::cerr << "Runtime error: " << e.what() << "\n";
            m_running = false; // exit
        }
        catch(const std::bad_alloc& e){
            std::cerr << "Allocation error: " << e.what() << "\n";
            m_running = false; // exit
        }
        catch (const std::exception& e) {
            std::cerr << "Standard exception: " << e.what() << "\n";
            m_running = false; // exit
        } 
        catch (...) {
            std::cerr << "Unknown exception caught \n";
            m_running = false; // exit
        }


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
