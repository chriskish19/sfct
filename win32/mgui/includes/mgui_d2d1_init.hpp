#pragma once
#include <d2d1.h>
#include <windows.h>



namespace mgui{
    class d2d1_init{
    public:
        d2d1_init(HWND WindowHandle);



    private:
        ID2D1Factory* m_pFactory{nullptr};
        ID2D1HwndRenderTarget* m_pRenderTarget{nullptr};
        ID2D1SolidColorBrush* m_pBrush{nullptr};



    };
}


