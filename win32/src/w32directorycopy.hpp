#pragma once
#include "w32cpplib.hpp"
#include "w32sfct_api.hpp"
#include "w32timer.hpp"
#include "w32benchmark.hpp"

//////////////////////////////////////////////////////////////////////////////
// This header contains directory_copy class.                               //
// This class handles copying directories and fast_copying directories      //
//////////////////////////////////////////////////////////////////////////////

namespace application{
    class directory_copy{
    public:
        /// @brief default destructor
        ~directory_copy()= default;
        
        /// @brief delete the Copy constructor
        directory_copy(const directory_copy&) = delete;

        /// @brief delete the Copy assignment operator
        directory_copy& operator=(const directory_copy&) = delete;

        /// @brief delete the Move constructor
        directory_copy(directory_copy&&) = delete;

        /// @brief delete the Move assignment operator
        directory_copy& operator=(directory_copy&&) = delete;

        /// @brief main constructor
        /// @param dirs the directories to copy
        directory_copy(std::shared_ptr<std::vector<copyto>> dirs) noexcept;

        /// @brief fast copy the directories
        void fast_copy() noexcept;
        
        /// @brief regular copy the directories
        void copy() noexcept;
    private:
        std::shared_ptr<std::vector<copyto>> m_dirs;
    };
}