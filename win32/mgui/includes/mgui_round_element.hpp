#pragma once
#include "w32cpplib.hpp"
#include "mgui_element.hpp"


namespace mgui{
    class round_element:public element{
    public:
        round_element();

    protected:
        float radius{};


    };
}