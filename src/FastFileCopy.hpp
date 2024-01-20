#pragma once
#include "appMacros.hpp"
#include <memory>
#include "obj.hpp"
#include <vector>
#include "logger.hpp"
#include "WinHelper.hpp"
#include <filesystem>
#include "Helper.hpp"

/////////////////////////////////////////////////////////////////////////////
/* This class will be OS specfic for more control over the copying process */
/* and it will be faster than FileCopy                                     */
/////////////////////////////////////////////////////////////////////////////

namespace application{
#if WINDOWS_BUILD
#include <Windows.h>
    class FastFileCopy{
    public:
        FastFileCopy(std::shared_ptr<std::vector<copyto>> dirs);

        void copy();
    private:
        void recursive(const copyto& dir);
        void single(const copyto& dir);

        std::shared_ptr<std::vector<copyto>> m_dirs;
    };
#endif
}