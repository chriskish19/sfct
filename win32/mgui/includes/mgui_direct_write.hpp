#pragma once
#include "w32cpplib.hpp"
#include "w32logger.hpp"
#include <dwrite.h>


namespace mgui{
    class direct_write{
    public:
        direct_write();
        ~direct_write();
    
        bool init_direct_write();
    
    
    protected:
        IDWriteFactory* m_pDWriteFactory{nullptr};
        IDWriteTextFormat* m_pTextFormat{nullptr};

        bool m_successful_init{false};
    };
}