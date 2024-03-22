#pragma once
#include "w32cpplib.hpp"
#include <d2d1.h>

namespace mgui{
    /// @brief Returns true if the point is inside the rectangle
    /// @param point The point to check
    /// @param rect The rectangle to check
    /// @return True if the point is inside the rectangle
    bool is_point_inside_rect(const D2D1_POINT_2F& point, const D2D1_RECT_F& rect);

    /// @brief Returns true if the point is inside the rectangle
    /// @param point The point to check
    /// @param rect The rectangle to check
    /// @return True if the point is inside the rectangle
    bool is_point_inside_rect(const POINT& point, const D2D1_RECT_F& rect);
}