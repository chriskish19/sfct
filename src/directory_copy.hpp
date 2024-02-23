#pragma once
#include "sfct_api.hpp"
#include "timer.hpp"

namespace application{
    class directory_copy{
    public:
        directory_copy(std::shared_ptr<std::vector<copyto>> dirs);

        void fast_copy();
        void copy();
    private:
        std::shared_ptr<std::vector<copyto>> m_dirs;
    };
}