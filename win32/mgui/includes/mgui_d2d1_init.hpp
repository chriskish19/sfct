#pragma once
#include <d2d1.h>
#include <windows.h>
#include "w32logger.hpp"



/**
 * The d2d1_init class is responsible for initializing Direct2D (D2D1) and creating a render target.
 * 
 * Direct2D is a graphics API provided by Microsoft that allows for efficient rendering of 2D graphics on Windows platforms. 
 * It provides a high-performance, hardware-accelerated rendering pipeline for tasks such as drawing shapes, text, and images.
 * 
 * The d2d1_init class encapsulates the initialization process for Direct2D and provides a convenient interface for creating a render target. 
 * The render target is a surface that represents the destination for drawing operations. It is used to draw graphics onto the screen or any other output device.
 * 
 * By using the d2d1_init class, you can easily set up the necessary resources for Direct2D and create a render target, 
 * which can then be used to perform various drawing operations in your application.
 */


namespace mgui {
    /// @brief Initializes d2d1
    /// @details Initializes d2d1 and creates a render target
    /// @details The render target is used to draw to the screen
    class d2d1_init {
    public:
        /// @brief Destructor
        /// @details Cleans up resources
        ~d2d1_init();

        /// @brief Copy constructor
        d2d1_init(const d2d1_init& other) = delete;

        /// @brief Move constructor
        d2d1_init(d2d1_init&& other) = delete;

        /// @brief Copy assignment operator
        d2d1_init& operator=(const d2d1_init& other) = delete;

        /// @brief Move assignment operator
        d2d1_init& operator=(d2d1_init&& other) = delete;

        /// @brief Main Constructor
        /// @details Initializes the d2d1 object and creates a render target
        /// @param WindowHandle The window handle
        d2d1_init(HWND WindowHandle);

        /// @brief Returns the factory
        /// @return The ID2D1Factory pointer
        ID2D1Factory* get_factory() const { return m_pFactory; }

        /// @brief Returns the render target
        /// @return The ID2D1HwndRenderTarget pointer
        ID2D1HwndRenderTarget* get_render_target() const { return m_pRenderTarget; }

        /// @brief Returns true if the d2d1 init was successful
        /// @return True if the d2d1 init was successful, false otherwise
        bool successful_d2d1_init() const { return m_successful_d2d1_init; }

    private:
        /// @brief Initializes d2d1
        /// @details Initializes the d2d1 object and creates a render target
        /// @return True if the d2d1 init was successful, false otherwise
        bool init_d2d1();

        /// @brief The factory 
        /// @details The factory is used to create other d2d resources                  
        ID2D1Factory* m_pFactory{ nullptr };

        /// @brief The render target
        /// @details The render target is used to draw to the screen
        ID2D1HwndRenderTarget* m_pRenderTarget{ nullptr };

        /// @brief True if the d2d1 init was successful
        bool m_successful_d2d1_init{ false };

        /// @brief The window handle
        HWND m_window_handle{ nullptr };
    };
}


