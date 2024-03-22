#pragma once
#include "w32cpplib.hpp"
#include "mgui_box_element.hpp"
#include <d2d1.h>
#include <windows.h>
#include "mgui_api.hpp"


namespace mgui{
    /// @brief A button element
    /// @details A button element that can be pressed and released
    /// @details The button element can be hovered over
    class box_button:public box_element{
    public:
        /// @brief Constructor
        box_button();

        /// @brief Returns the shared pointer to the pressed state
        /// @return The shared pointer to the pressed state
        const std::shared_ptr<std::atomic<bool>> get_hovering_sp(){return m_hovering;}

        /// @brief Returns the shared pointer to the pressed state
        /// @return The shared pointer to the pressed state
        const std::shared_ptr<std::atomic<bool>> get_pressed_sp(){return m_pressed;}

        /// @brief Returns true if the button is pressed
        /// @return True if the button is pressed
        bool is_pressed() const;

        /// @brief Returns true if the button is hovering over
        /// @return True if the button is hovering over
        bool is_hovering_over();
        
        /// @brief Presses the button
        void manual_press();

        /// @brief Releases the button
        void manual_release();

    protected:
        
        /// @brief Called when the button is pressed
        std::shared_ptr<std::atomic<bool>> m_pressed{std::make_shared<std::atomic<bool>>(false)};

        /// @brief Called when the button is hovering over
        std::shared_ptr<std::atomic<bool>> m_hovering{std::make_shared<std::atomic<bool>>(false)};
    };
}