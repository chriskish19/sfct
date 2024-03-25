#pragma once
#include "w32cpplib.hpp"
#include <d2d1.h>
#include <windows.h>


namespace mgui{
    class element{
    public:
        element();


        /// @brief Returns the shared pointer to the pressed state
        /// @return The shared pointer to the pressed state
        const std::shared_ptr<std::atomic<bool>> get_hovering_sp(){return m_hovering;}

        /// @brief Returns the shared pointer to the pressed state
        /// @return The shared pointer to the pressed state
        const std::shared_ptr<std::atomic<bool>> get_pressed_sp(){return m_pressed;}

        /// @brief checks if the element was pressed
        /// @return True if the element was pressed
        virtual bool is_pressed() const = 0;

        /// @brief Returns true if the element is being hovered over
        /// @return True if selector(using mouse or keyboard) is hovering over the element
        virtual bool is_hovering_over() = 0;
    protected:
        // color to fill, default is Aqua if blank constructor is used
        D2D1::ColorF color{D2D1::ColorF::Aqua};
        
        // text on the element
        std::wstring text{};

        // the window where the element resides
        HWND m_window_handle{nullptr};

        /// @brief Called when the element is pressed
        std::shared_ptr<std::atomic<bool>> m_pressed{std::make_shared<std::atomic<bool>>(false)};

        /// @brief Called when the element is being hovered over
        std::shared_ptr<std::atomic<bool>> m_hovering{std::make_shared<std::atomic<bool>>(false)};
    };
}