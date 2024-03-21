#include "mgui_d2d1_init.hpp"

mgui::d2d1_init::d2d1_init(HWND WindowHandle)
{
    // Initialize Direct2D factory
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);

    // Create render target
    RECT rc;
    GetClientRect(WindowHandle, &rc);
    m_pFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(
            WindowHandle,
            D2D1::SizeU(static_cast<UINT>(rc.right - rc.left), static_cast<UINT>(rc.bottom - rc.top))
        ),
        &m_pRenderTarget
    );
}