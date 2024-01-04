#pragma once
#include <thread>
#include <vector>
#include "ConsoleTM.hpp"
#include <utility>


//////////////////////////////////////////////////////////////////
/*Future TODO:                                                  */
/* 1. Implement std::async so functions can return values       */
/* 2. Change do_work() to accept function args                  */
//////////////////////////////////////////////////////////////////

namespace application{
    enum class SystemPerformance{
        SLOW,
        AVERAGE,
        FAST
    };
    

    // thread manager class
    // a small wrapper to manage thread creation and work
    class TM{
    public:
        // default constructor
        // sets the number of workers to use
        TM();

        // joins all joinable threads
        // causes main thread to wait until all threads finish work
        void join_all();

        // joins the first launched thread from do_work
        // this is used to keep the number of working threads (m_Workers) continuously
        // working when one thread has finished its work a new thread is launched, the total
        // working threads will be m_Workers at any given time
        void join_one();

        // jobs to do for the threads
        template <typename Function, typename... Args>
        void do_work(Function fp, Args&&... args) {
            if (m_Threads.size() < m_Workers) {
                m_Threads.emplace_back(fp, std::forward<Args>(args)...);
            }
        }

        // returns the number of workers used by TM class specified by the pc spec
        size_t GetNumberOfWorkers(){return m_Workers;}
    private:
        // set the number of worker to use
        void SetWorkers();
        
        // workers to initialize
        size_t m_Workers;

        // avaliable threads
        const inline static unsigned int m_TotalThreads{std::thread::hardware_concurrency()};

        // holds the worker threads
        std::vector<std::thread> m_Threads;

        // using the total threads, it determines the SystemPerformance enum
        static SystemPerformance GetPCspec() noexcept;

        // if the system is slow then copy operations are slowed down
        // if the system is considered fast then copy operations are not slowed down
        const inline static SystemPerformance m_PC_SPEC{GetPCspec()};
    };
}