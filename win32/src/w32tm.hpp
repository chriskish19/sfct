#pragma once
#include <thread>
#include <vector>
#include <utility>
#include <exception>
#include <functional>
#include <filesystem>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////////////////////////////
// This header is responsible for managing threads, its a wrapper for std::thread.                    //
// It checks how many threads are available and determines if the system is slow, average or fast.    //
// It then sets a maximum number of threads to use at one time.                                       //
// It prevents high cpu usage                                                                         //
//                                                                                                    //
// Future TODO:                                                                                       //
// 1. implement std::asyc so functions can return values.                                             //
////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace application{
    /// @brief a wrapper for threads to call so exceptions are handled by the thread that was launched.
    /// @tparam Function 
    /// @tparam ...Args 
    /// @param fp function pointer
    /// @param ...args 
    template <typename Function, typename... Args>
    std::exception_ptr exceptions(Function fp, Args&&... args){
        std::exception_ptr eptr;
    
        try {
            // Call the function with the provided arguments
            std::invoke(fp, std::forward<Args>(args)...);
        } catch (...) {
            // Capture any exception into an std::exception_ptr
            eptr = std::current_exception();
            // Optionally log that an exception was captured
            std::cerr << "Exception captured \n";
        }

        // Return the exception pointer for further handling
        return eptr;
    }

    enum class SystemPerformance{
        SLOW,
        AVERAGE,
        FAST
    };
    
    /// @brief thread manager class
    class TM{
    public:
        /// @brief default destructor
        ~TM()= default;
        
        /// @brief Copy constructor
        TM(const TM&) = delete;

        /// @brief Copy assignment operator
        TM& operator=(const TM&) = delete;

        /// @brief Move constructor
        TM(TM&&) = delete;

        /// @brief Move assignment operator
        TM& operator=(TM&&) = delete;


        /// @brief default constructor
        /// sets the number of workers to use
        TM() noexcept;

        /// @brief joins all joinable threads
        /// causes main thread to wait until all threads finish work
        void join_all() noexcept;

        /// @brief joins the first launched thread from do_work
        /// this is used to keep the number of working threads (m_Workers) continuously
        /// working when one thread has finished its work a new thread is launched, the total
        /// working threads will be m_Workers at any given time
        /// @return true if the oldest thread was joined and false if not
        bool join_one() noexcept; 

        /// @brief jobs to do for the threads wrapped in the exceptions function
        /// @tparam Function 
        /// @tparam ...Args 
        /// @param fp function pointer
        /// @param ...args arguments that match the function pointer
        template <typename Function, typename... Args>
        void do_work_exceptions(Function fp, Args&&... args) {
            if (m_Threads.size() < m_Workers) {
                // Start a new thread that calls the exceptions function with fp and args...
                m_Threads.emplace_back([fp, args...](){
                    exceptions(fp, std::forward<Args>(args)...);
                });
            }
        }

        /// @brief jobs to do for the threads
        /// @tparam Function 
        /// @tparam ...Args 
        /// @param fp function pointer
        /// @param ...args arguments that match the function pointer
        template <typename Function, typename... Args>
        void do_work(Function fp, Args&&... args) noexcept {
            if (m_Threads.size() < m_Workers) {
                try{
                    // Start a new thread that calls the exceptions function with fp and args...
                    m_Threads.emplace_back(fp,std::forward<Args>(args)...);
                }
                catch (const std::filesystem::filesystem_error& e) {
                    // Handle filesystem related errors
                    std::cerr << "Filesystem error: " << e.what() << "\n";
                }
                catch(const std::runtime_error& e){
                    // the error message
                    std::cerr << e.what() << "\n";
                }
                catch(const std::bad_alloc& e){
                    // the error message
                    std::cerr << e.what() << "\n";
                }
                catch (const std::exception& e) {
                    // Catch other standard exceptions
                    std::cerr << "Standard exception: " << e.what() << "\n";
                } catch (...) {
                    // Catch any other exceptions
                    std::cerr << "Unknown exception caught \n";
                }
            }
        }


        /// @brief returns the number of workers used by TM class specified by the pc spec
        /// @return number of worker threads
        size_t GetNumberOfWorkers() noexcept {return m_Workers;}
    private:
        /// @brief set the number of worker to use
        void SetWorkers() noexcept;
        
        /// @brief workers to initialize
        size_t m_Workers;

        /// @brief total avaliable threads
        const inline static unsigned int m_TotalThreads{std::thread::hardware_concurrency()};

        /// @brief holds the worker threads
        std::vector<std::jthread> m_Threads;

        /// @brief using the total threads, it determines the SystemPerformance enum
        /// @return SystemPerformance enum value which sets the amount of threads to use at any one time
        static SystemPerformance GetPCspec() noexcept;

        /// @brief if the system is slow then copy operations are slowed down
        // if the system is considered fast then copy operations are not slowed down
        const inline static SystemPerformance m_PC_SPEC{GetPCspec()};
    };
}
