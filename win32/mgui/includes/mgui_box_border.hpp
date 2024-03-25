#pragma once
#include <d2d1.h>
#include <windows.h>
#include "w32cpplib.hpp"
#include "mgui_box_element.hpp"




namespace mgui {
    class box_border: public box_element {
    public:
        box_border(D2D1::ColorF border_color, float thickness);

        void draw() override;
    protected:
        D2D1::ColorF m_border_color{D2D1::ColorF::Black};
        float m_thickness{1.0f};
    };
} 
