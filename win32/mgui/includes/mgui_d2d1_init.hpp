#pragma once
#include <d2d1.h>
#include <windows.h>
#include "w32logger.hpp"


namespace mgui{
    class d2d1_init{
    public:
        d2d1_init(HWND WindowHandle);

        bool init_d2d1();

    private:
        ID2D1Factory* m_pFactory{nullptr};
        ID2D1HwndRenderTarget* m_pRenderTarget{nullptr};
        ID2D1SolidColorBrush* m_pBrush{nullptr};

        bool m_successful_d2d1_init{false};


        HWND m_window_handle{nullptr};
    };
}


