#include "mgui_d2d1_init.hpp"

mgui::d2d1_init::~d2d1_init()
{
    // Release resources
    if(m_pFactory){
        m_pFactory->Release();
    }

    if(m_pRenderTarget){
        m_pRenderTarget->Release();
    }
}

mgui::d2d1_init::d2d1_init(HWND WindowHandle)
    : m_window_handle(WindowHandle)
{
    m_successful_d2d1_init = init_d2d1();
}

bool mgui::d2d1_init::init_d2d1()
{
    {
        // Initialize Direct2D factory
        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);
        if(FAILED(hr)){
            application::logger log(application::Error::FATAL,hr);
            log.to_console();
            log.to_log_file();
            log.to_output();
            return false;
        }
    }


    // Create render target
    RECT rc;
    if(!GetClientRect(m_window_handle, &rc)){
        application::logger log(application::Error::FATAL);
        log.to_console();
        log.to_log_file();
        log.to_output();
        return false;
    }
    


    {
        HRESULT hr = m_pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                m_window_handle,
                D2D1::SizeU(static_cast<UINT>(rc.right - rc.left), static_cast<UINT>(rc.bottom - rc.top))
            ),
            &m_pRenderTarget
        );

        if(FAILED(hr)){
            application::logger log(application::Error::FATAL,hr);
            log.to_console();
            log.to_log_file();
            log.to_output();
            return false;
        }
    }

    return true;

}
