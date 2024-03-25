#pragma once
#include "w32cpplib.hpp"
#include <d2d1.h>
#include <windows.h>

namespace mgui{
    /// @brief The selector class is used to select elements in the gui using the mouse or keyboard
    class selector{
    public:
        selector();



    protected:
        // the window where the selector resides
        HWND m_window_handle{nullptr};


    };
}