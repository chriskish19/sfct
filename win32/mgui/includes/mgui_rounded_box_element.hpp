#pragma once
#include <d2d1.h>
#include <windows.h>
#include "mgui_element.hpp"

namespace mgui {

    class rounded_box_element :public element{
    public:
        rounded_box_element();

        void draw() override;
    protected:
        

    };

} 

