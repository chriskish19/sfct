#pragma once
#include <filesystem>
#include "TM.hpp"
#include "FileParse.hpp"
#include "logger.hpp"
#include <mutex>
#include <atomic>
#include <unordered_set>

//////////////////////////////////////////////////////
/* How this is supposed to work:                    */
/* 1. Create a FileCopyState object shared_ptr      */
/* 2. */
//////////////////////////////////////////////////////



namespace application{
    // global variables used in multi threading the FileCopy class
    // I made these global to avoid using class objects which are not thread safe
    // to access
    namespace MT{
        // used in copy_file function
        inline std::atomic<bool> running = true;

        // used to prevent concurrent access on shared resources in next_file()
        // each thread must wait in line to access the next file
        inline std::mutex m_next_file_mtx;

        // prevents copy file being called when no more entrys exist
        inline std::atomic<bool> no_files_left = false;
    }
    
    class FileCopyState{
    public:
        friend class FileCopy;

        FileCopyState(const std::shared_ptr<std::vector<copyto>> pDirectories,const std::filesystem::copy_options co);

    private:
        // flags for copying
        const std::filesystem::copy_options m_CO;

        // the list of directories to copy from and to
        const std::shared_ptr<std::vector<copyto>> m_pDirectories;

        std::filesystem::directory_iterator m_dit;
        std::filesystem::directory_iterator m_ditend{std::filesystem::directory_iterator()};

        std::filesystem::recursive_directory_iterator m_rdit;
        std::filesystem::recursive_directory_iterator m_rdendit{std::filesystem::recursive_directory_iterator()};

        size_t m_Directories_index{};

        // prevent a cycle of infinite looping through the directories
        // if there is symbolic links that create loops
        std::unordered_set<std::filesystem::path> m_visited_directories;

        // recursive directory iterator initialized boolean flag
        bool m_rdit_init{false};

        // directory iterator initalized boolean flag
        bool m_dit_init{false};
    };
    
    // Thread safe class for copying files in directories
    // To use create a FileCopy object for each worker thread
    class FileCopy{
    public:
        FileCopy(const std::shared_ptr<FileCopyState> pState);

        // main copy file function
        // it is thread safe
        void copy_file();

        // thread safe
        const copyto GetCurrentCopyDirectory() noexcept {return m_CurrentDirectory;}
    private:
        // not thread safe to access
        const std::shared_ptr<FileCopyState> m_pState;

        // gets the next file ready to be copied
        // thread safe
        const copyto next_file();

        // copy of the flag options, thread safe
        std::filesystem::copy_options m_co;

        // copy of current file directory being copied, thread safe
        copyto m_CurrentDirectory;

        // prevent multiple threads running on the same FileCopy object
        std::mutex m_copy_file_mtx;
    };
}