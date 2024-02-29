#pragma once
#include "sfct_api.hpp"
#include "timer.hpp"
#include "benchmark.hpp"


namespace application{
    class directory_copy{
    public:
        // default destructor
        ~directory_copy()= default;
        
        // Copy constructor
        directory_copy(const directory_copy&) = delete;

        // Copy assignment operator
        directory_copy& operator=(const directory_copy&) = delete;

        // Move constructor
        directory_copy(directory_copy&&) = delete;

        // Move assignment operator
        directory_copy& operator=(directory_copy&&) = delete;

        directory_copy(std::shared_ptr<std::vector<copyto>> dirs) noexcept;

        void fast_copy() noexcept;
        void copy() noexcept;
    private:
        std::shared_ptr<std::vector<copyto>> m_dirs;
    };
}