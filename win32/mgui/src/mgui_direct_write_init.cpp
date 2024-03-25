#include "mgui_direct_write_init.hpp"

mgui::direct_write_init::direct_write_init()
{
    // Initialize direct write
    m_successful_init = init_direct_write();
}

mgui::direct_write_init::~direct_write_init()
{
    // clean up resources
    if (m_pDWriteFactory) {                 // If the factory exists
        m_pDWriteFactory->Release();        // Release the factory
        m_pDWriteFactory = nullptr;         // Set the factory to nullptr
    }

}

bool mgui::direct_write_init::init_direct_write()
{
    // Initialize DirectWrite
    HRESULT hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,     // Shared factory
        __uuidof(IDWriteFactory),       // The interface to create
        reinterpret_cast<IUnknown**>(&m_pDWriteFactory) // The factory
        );

    if(FAILED(hr)){
        // log the error
        application::logger log(application::Error::FATAL,hr);
        log.to_console();
        log.to_log_file();
        log.to_output();
        return false;
    }
    
    // return true if successful
    return true;
}
