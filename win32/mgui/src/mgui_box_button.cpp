#include "mgui_box_button.hpp"

mgui::box_button::box_button()
{

}

mgui::box_button::box_button(std::function<void()> callback)
:m_callback(callback)
{

}

void mgui::box_button::press()
{
    // Store true in m_pressed
    m_pressed->store(true);

    // Call the callback function
    if(m_callback){
        m_callback();
    }
}

void mgui::box_button::release()
{
    // Store false in m_pressed
    m_pressed->store(false);
}


