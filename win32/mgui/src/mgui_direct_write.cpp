#include "mgui_direct_write.hpp"

mgui::direct_write::direct_write()
{
    m_successful_init = init_direct_write();
}

mgui::direct_write::~direct_write()
{
    // clean up
    if (m_pTextFormat) {
        m_pTextFormat->Release();
        m_pTextFormat = nullptr;
    }

    if (m_pDWriteFactory) {
        m_pDWriteFactory->Release();
        m_pDWriteFactory = nullptr;
    }

}

bool mgui::direct_write::init_direct_write()
{
    {
        HRESULT hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
        );

        if(FAILED(hr)){
            application::logger log(application::Error::FATAL,hr);
            log.to_console();
            log.to_log_file();
            log.to_output();
            return false;
        }
    }



    {
        HRESULT hr = m_pDWriteFactory->CreateTextFormat(
            L"Segoe UI",                    // Font family name
            NULL,                           // Font collection (NULL sets it to the system font collection)
            DWRITE_FONT_WEIGHT_NORMAL,      // Font weight
            DWRITE_FONT_STYLE_NORMAL,       // Font style
            DWRITE_FONT_STRETCH_NORMAL,     // Font stretch
            24.0f,                          // Font size
            L"en-us",                       // Locale
            &m_pTextFormat                  // Pointer to the IDWriteTextFormat object
        );

        if(FAILED(hr)){
            application::logger log(application::Error::FATAL,hr);
            log.to_console();
            log.to_log_file();
            log.to_output();
            return false;
        }
    }




    {
        HRESULT hr = m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        if(FAILED(hr)){
            application::logger log(application::Error::FATAL,hr);
            log.to_console();
            log.to_log_file();
            log.to_output();
            return false;
        }
    }
    

    {
        HRESULT hr = m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        if(FAILED(hr)){
            application::logger log(application::Error::FATAL,hr);
            log.to_console();
            log.to_log_file();
            log.to_output();
            return false;
        }
    }

    
    // if everything passes and gets created return true
    return true;
}
