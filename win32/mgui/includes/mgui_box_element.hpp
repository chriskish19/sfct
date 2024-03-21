#pragma once
#include "w32cpplib.hpp"
#include <d2d1.h>
#include <dwrite.h>

namespace mgui{
    class box_element{
    public:
        box_element();
        box_element(int x,int y, int width,int height);

    protected:
        // left, top, right, bottom
        std::shared_ptr<D2D1_RECT_F> dimensions{std::make_shared<D2D1_RECT_F>(0.0f,0.0f,0.0f,0.0f)};

        // color to fill
        D2D1::ColorF color;
        
        // text on the element
        std::wstring text{};


    };
}