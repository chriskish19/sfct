#include "mgui_box_element.hpp"

mgui::box_element::box_element()
{

}

mgui::box_element::box_element(int x, int y, int width, int height)
{
    m_dimensions->left = x;
    m_dimensions->top = y;
    m_dimensions->bottom = width;
    m_dimensions->top = height;
}

bool mgui::box_element::is_pressed() const
{
    return m_pressed->load();
}

bool mgui::box_element::is_hovering_over()
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
