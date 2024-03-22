#include "mgui_box_button.hpp"

mgui::box_button::box_button()
{

}

bool mgui::box_button::is_pressed() const
{
    // Return the value of m_pressed
    return m_pressed->load();
}

bool mgui::box_button::is_hovering_over()
{
    // used to get the current mouse position
    POINT current_mouse_position;

    // Get the current mouse position
    GetCursorPos(&current_mouse_position);
    
    // Converts the screen coordinates to client coordinates
    // The client coordinates are relative to the window's client area
    ScreenToClient(m_window_handle, &current_mouse_position);

    // current_mouse_position.x and current_mouse_position.y now contain the mouse coordinates relative to the window's client area
    bool result = is_point_inside_rect(current_mouse_position,*m_dimensions);

    // Store the result in m_hovering
    m_hovering->store(result);

    // Return the result
    return result;
}

void mgui::box_button::manual_press()
{
    // Store true in m_pressed
    m_pressed->store(true);
}

void mgui::box_button::manual_release()
{
    // Store false in m_pressed
    m_pressed->store(false);
}


