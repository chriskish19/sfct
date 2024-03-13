#pragma once
#include "w32cpplib.hpp"
#include "w32constants.hpp"
#include "w32sfct_api.hpp"

//////////////////////////////////////////////////////////////////
// This header contains the class benchmark.                    //
// It is useful for testing the copy rate of files in MB/s      //
//////////////////////////////////////////////////////////////////


namespace application{
    class benchmark{
    public:
        /// @brief default constructor
        benchmark() = default;

        /// @brief default destructor
        ~benchmark()= default;
        
        /// @brief delete the Copy constructor
        benchmark(const benchmark&) = delete;

        /// @brief delete the Copy assignment operator
        benchmark& operator=(const benchmark&) = delete;

        /// @brief delete the Move constructor
        benchmark(benchmark&&) = delete;

        /// @brief delete the Move assignment operator
        benchmark& operator=(benchmark&&) = delete;   

        /// @brief resets the members m_start, m_end and m_duration by calling blank constructors
        /// call this function if reusing the same benchmark object for calculating speed().
        void reset_clock() noexcept;

        /// @brief call this function to begin the clock.
        /// uses std::chrono time_point.
        /// use this function and end_clock() together with speed() to obtain a rate
        /// of transfer when copying files
        void start_clock() noexcept;

        /// @brief call this function to end the clock
        /// uses std::chrono time_point
        void end_clock() noexcept;

        /// @brief calculates the rate in MB/s
        /// must call start_clock() and end_clock() before and after the transfer operation to get a valid rate
        /// If reusing the same benchmark object for calculating speed() you must call reset_clock()
        /// @param totalSize size in bytes of the data being transfered
        /// @return the rate in MB/s
        double_t speed(std::uintmax_t totalSize) noexcept;

        /// @brief copies a single file from one location to another and calculates it transfer speed in MB/s
        /// @param dir the directory location copied from and to
        /// @param bytes the size of the file in bytes
        void speed_test(const copyto& dir,std::uintmax_t bytes) noexcept;

        /// @brief copies a number of files set by filesCount and the total bytes set by bytes
        /// useful for speed testing the copy rate of many small files
        /// @param dir the directory location copied from and to
        /// @param filesCount the count of the files
        /// @param bytes the total size of all files in bytes
        void speed_test_4k(const copyto& dir,std::uintmax_t filesCount,std::uintmax_t bytes) noexcept;

        /// @brief speed test a vector of directories. For loop through the vector either calling regular test or 4k test depending
        /// on the cs commands in the copyto objects
        /// @param dirs a vector of directories to speed test. Uses cs commands to specify 4k test or regular speed_test.
        void speed_test_directories(const std::vector<copyto>& dirs) noexcept;
    private:
        /// @brief start point and end point times
        /// set m_start by calling start_clock()
        /// set m_end by calling end_clock()
        std::chrono::steady_clock::time_point m_start,m_end;

        /// @brief the calculation of m_start and m_end time
        std::chrono::duration<double_t> m_duration;
    };
}