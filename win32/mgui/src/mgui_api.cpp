#include "mgui_api.hpp"

bool mgui::is_point_inside_rect(const D2D1_POINT_2F &point, const D2D1_RECT_F &rect)
{
    // The point is inside the rectangle if the point's x and y coordinates are
    // greater than or equal to the rectangle's left and top coordinates, and
    // less than or equal to the rectangle's right and bottom coordinates.
    return point.x >= rect.left &&
           point.x <= rect.right &&
           point.y >= rect.top &&
           point.y <= rect.bottom;
}

bool mgui::is_point_inside_rect(const POINT &point, const D2D1_RECT_F &rect)
{
    // Convert the point's x and y coordinates to float
    // so that we can use the is_point_inside_rect function
    // that takes D2D1_POINT_2F as the first argument.
    float x{static_cast<float>(point.x)};
    float y{static_cast<float>(point.y)};

    // The point is inside the rectangle if the point's x and y coordinates are
    // greater than or equal to the rectangle's left and top coordinates, and
    // less than or equal to the rectangle's right and bottom coordinates.
    return x >= rect.left &&
           x <= rect.right &&
           y >= rect.top &&
           y <= rect.bottom;
}
