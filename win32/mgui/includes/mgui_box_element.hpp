#pragma once
#include "w32cpplib.hpp"            
#include "mgui_element.hpp"    
#include <d2d1.h>


namespace mgui{
    /// @brief A box element
    /// @details A box element that has dimensions
    /// @details The box element has a left, top, right, and bottom
    class box_element:public element{
    public:
        /// @brief Constructor
        box_element();

        /// @brief Constructor
        /// @param x The x coordinate
        /// @param y The y coordinate
        /// @param width The width
        /// @param height The height
        box_element(int x,int y, int width,int height);



        std::shared_ptr<D2D1_RECT_F> get_dimensions_rect_sp(){return m_dimensions;}
    protected:
        // left, top, right, bottom
        std::shared_ptr<D2D1_RECT_F> m_dimensions{std::make_shared<D2D1_RECT_F>(0.0f,0.0f,0.0f,0.0f)};

        


    };
}