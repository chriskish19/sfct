#pragma once
#include "sfct_api.hpp"
#include "timer.hpp"
#include "benchmark.hpp"


namespace application{
    class directory_copy{
    public:
        directory_copy(std::shared_ptr<std::vector<copyto>> dirs) noexcept;

        void fast_copy();
        void copy();
    private:
        std::shared_ptr<std::vector<copyto>> m_dirs;
    };
}