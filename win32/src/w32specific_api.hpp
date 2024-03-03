#pragma once
#include "w32cpplib.hpp"
#include "w32logger.hpp"
#include "w32appmacros.hpp"
#include <windows.h>


/////////////////////////////////////////////////////////////////////////////////
// This header contains windows specific functions                             //
/////////////////////////////////////////////////////////////////////////////////


namespace Windows{
    /// @brief this function is not being used until i figure out a way to handle large files, currently causes high ram usage.
    /// @param src any path
    /// @param dst any path
    /// @return true if successful copy operation, false if there was an error
    bool FastCopy(const std::filesystem::path src, const std::filesystem::path dst) noexcept;
}
