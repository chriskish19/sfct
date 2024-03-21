#pragma once
#include "w32cpplib.hpp"
#include "mgui_box_element.hpp"
#include <d2d1.h>


namespace mgui{
    class box_button:public box_element{
    public:
        box_button();
        




        std::shared_ptr<bool> get_hovering_sp(){return m_hovering;}
        std::shared_ptr<bool> get_pressed_sp(){return m_pressed;}


    protected:
        bool is_pressed();
        bool hover_over();

        std::shared_ptr<bool> m_pressed{std::make_shared<bool>(false)};
        std::shared_ptr<bool> m_hovering{std::make_shared<bool>(false)};
    };
}