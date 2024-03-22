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
