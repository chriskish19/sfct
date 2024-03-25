#pragma once
#include "w32cpplib.hpp"
#include <d2d1.h>
#include <windows.h>


namespace mgui{
    class element{
    public:
        /// @brief Constructor
        element();

        /// @brief Constructor
        /// @param WindowHandle The window handle
        element(HWND WindowHandle);

        /// @brief Destructor
        /**
         * The `virtual` keyword in C++ is used to declare a member function as virtual.
         * In the provided code snippet, the `virtual` keyword is used to declare the destructor of the `element` class as virtual.
         * 
         * When a function is declared as virtual, it allows the function to be overridden in derived classes.
         * This means that if a derived class inherits from the `element` class and provides its own implementation of the destructor,
         * the derived class's destructor will be called instead of the base class's destructor when an object of the derived class is destroyed.
         * 
         * In the context of the `element` class, making the destructor virtual is important when dealing with polymorphism.
         * If you have a pointer to the base class `element` that actually points to an object of a derived class,
         * and you call `delete` on that pointer, having a virtual destructor ensures that the destructor of the derived class is called first,
         * followed by the destructor of the base class. This ensures that all resources allocated by the derived class are properly cleaned up.
         * 
         * In summary, making the destructor virtual in the `element` class allows for proper destruction of derived class objects
         * when they are deleted through a base class pointer.
         */
        virtual ~element() = default;


        /// @brief Returns the shared pointer to the pressed state
        /// @return The shared pointer to the pressed state
        const std::shared_ptr<std::atomic<bool>> get_hovering_sp() const{return m_hovering;}

        /// @brief Returns the shared pointer to the pressed state
        /// @return The shared pointer to the pressed state
        const std::shared_ptr<std::atomic<bool>> get_pressed_sp() const{return m_pressed;}

        /// @brief checks if the element was pressed
        /// @return True if the element was pressed
        virtual bool is_pressed() const = 0;

        /// @brief Returns true if the element is being hovered over
        /// @return True if selector(using mouse or keyboard) is hovering over the element
        virtual bool is_hovering_over() = 0;

        /// @brief draws the element
        virtual void draw() = 0;
    protected:
        /// @brief color to fill, default is Aqua if blank constructor is used
        D2D1::ColorF color{D2D1::ColorF::Aqua};
        
        /// @brief text on the element
        std::wstring text{};

        /// @brief the window where the element resides
        HWND m_window_handle{nullptr};

        /// @brief changed when the element is pressed
        std::shared_ptr<std::atomic<bool>> m_pressed{std::make_shared<std::atomic<bool>>(false)};

        /// @brief changed when the element is being hovered over
        std::shared_ptr<std::atomic<bool>> m_hovering{std::make_shared<std::atomic<bool>>(false)};

        /// @brief The render target
        ID2D1HwndRenderTarget* m_render_target{nullptr};

        
    };
}