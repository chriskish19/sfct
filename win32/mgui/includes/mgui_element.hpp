#pragma once
#include "w32cpplib.hpp"
#include <d2d1.h>
#include <windows.h>


namespace mgui{
    class element{
    public:
        element();


    protected:
        // color to fill, default is Aqua if blank constructor is used
        D2D1::ColorF color{D2D1::ColorF::Aqua};
        
        // text on the element
        std::wstring text{};

        // the window where the element resides
        HWND m_window_handle{nullptr};


    };
}