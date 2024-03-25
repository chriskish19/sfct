#pragma once
#include "w32cpplib.hpp"
#include "mgui_box_element.hpp"
#include <d2d1.h>
#include <windows.h>
#include "mgui_api.hpp"


namespace mgui{
    /// @brief A rectangular button
    class box_button:public box_element{
    public:
        /// @brief Constructor
        box_button();

        /// @brief Constructor
        box_button(std::function<void()> callback);

        /// @brief Presses the button
        void press();

        /// @brief Releases the button
        void release();

        /// @brief Define a member function to set the callback function
        /// @param callback The callback function to set, so when the button is pressed this function is called
        void set_callback(std::function<void()> callback){m_callback = callback;}   

    protected:


        /// @brief Define a member variable to store the callback function
        std::function<void()> m_callback; 
        
    };
}