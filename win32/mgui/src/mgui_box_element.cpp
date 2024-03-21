#include "mgui_box_element.hpp"

mgui::box_element::box_element()
{

}

mgui::box_element::box_element(int x, int y, int width, int height)
{
    dimensions->left = x;
    dimensions->top = y;
    dimensions->bottom = width;
    dimensions->top = height;
}
