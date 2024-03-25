#pragma once
#include "w32cpplib.hpp"
#include "w32logger.hpp"
#include <dwrite.h>



/**
 * The direct_write_init class is responsible for initializing DirectWrite, 
 * which is a text layout and rendering API provided by Microsoft.
 * 
 * This class ensures that DirectWrite is properly initialized and ready to be used in the application. 
 * It encapsulates the initialization process and provides a convenient interface for managing the 
 * initialization and destruction of DirectWrite resources.
 * 
 * The class has a few important characteristics:
 * 
 * - It is designed to be non-copyable and non-movable. 
 *   This means that instances of the direct_write_init class cannot be copied or moved, 
 *   preventing unintended duplication or resource leaks.
 * 
 * - It has a default constructor, which is responsible for initializing DirectWrite. 
 *   This constructor is called when an instance of the class is created.
 * 
 * - It also has a destructor, which is responsible for cleaning up any resources allocated 
 *   during the initialization process. 
 *   The destructor is automatically called when an instance of the class goes out of scope or is explicitly 
 *   destroyed.
 * 
 * By encapsulating the initialization and destruction logic within this class, 
 * it provides a clean and organized way to manage DirectWrite resources in the application. 
 * This helps ensure that DirectWrite is properly initialized and ready to be used, 
 * and also guarantees that any allocated resources are properly released when they are no longer needed.
 */

namespace mgui{
    /// @brief Initializes direct write
    class direct_write_init{
    public:
        /// @brief Copy constructor (deleted)
        direct_write_init(const direct_write_init&) = delete;

        /// @brief Copy assignment operator (deleted)
        direct_write_init& operator=(const direct_write_init&) = delete;

        /// @brief Move constructor (deleted)
        direct_write_init(direct_write_init&& other) = delete;

        /// @brief Move assignment operator (deleted)
        direct_write_init& operator=(direct_write_init&& other) = delete;

        /// @brief Constructor
        direct_write_init();

        /// @brief Destructor
        ~direct_write_init();

        /// @brief Initializes direct write
        bool init_direct_write();

        /// @brief Returns true if the direct write init was successful
        bool successful_direct_write_init() const{return m_successful_init;}

        /// @brief Returns the factory
        IDWriteFactory* get_factory() const{return m_pDWriteFactory;}
    protected:
        /// @brief The text format
        IDWriteFactory* m_pDWriteFactory{nullptr};

        /// @brief True if the direct write init was successful
        bool m_successful_init{false};
    };
}