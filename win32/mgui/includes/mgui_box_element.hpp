#pragma once
#include "w32cpplib.hpp"
#include "mgui_element.hpp"
#include <d2d1.h>


namespace mgui{
    class box_element:public element{
    public:
        box_element();
        box_element(int x,int y, int width,int height);

    protected:
        // left, top, right, bottom
        std::shared_ptr<D2D1_RECT_F> dimensions{std::make_shared<D2D1_RECT_F>(0.0f,0.0f,0.0f,0.0f)};

        


    };
}