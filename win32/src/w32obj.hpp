#pragma once
#include "w32cpplib.hpp"
#include "w32args.hpp"


//////////////////////////////////////////////////////////////////////////////////
// This header contains common structures that are used throughout the program. //
//////////////////////////////////////////////////////////////////////////////////


namespace application{
    // used as the main struct for holding the directories to be proccessed
    struct copyto{
        std::filesystem::path source;                                               // source directory path to monitor or copy from 
        std::filesystem::path destination;                                          // destination directory path to copy or sync to         
        cs commands = cs::none;                                                     // holds the arguments whether to copy or monitor, sync, update ect.                   
        std::filesystem::copy_options co = std::filesystem::copy_options::none;     // holds the arguments in std::filesystem::copy_options format       
    };

    inline bool copyto_equal(const copyto& a, const copyto& b){
        // Define what makes two copyto objects equal
        return a.source == b.source && a.destination == b.destination && a.commands == b.commands && a.co == b.co;
    }

    inline bool copyto_comparison(const copyto& a, const copyto& b){
        // Define ordering for copyto objects
        if (a.source != b.source) {
            return a.source < b.source;
        }
        if (a.destination != b.destination) {
            return a.destination < b.destination;
        }
        if (a.commands != b.commands) {
            return a.commands < b.commands;
        }
            
        return a.co < b.co;
    }

    struct directory_info{
        std::uintmax_t TotalSize;           // total size of the directory in bytes
        std::uintmax_t FileCount;           // number of files in the directory
        double_t AvgFileSize;               // average file size in bytes

         // Overload the += operator
        directory_info& operator+=(const directory_info& other) {
            // Add the TotalSize and FileCount from the other object to this one
            TotalSize += other.TotalSize;
            FileCount += other.FileCount;

            // Recalculate the average file size
            // Check for division by zero
            if (FileCount > 0) {
                AvgFileSize = static_cast<double_t>(TotalSize) / FileCount;
            } else {
                AvgFileSize = 0;
            }

            // Return a reference to this object
            return *this;
        }
    };

    // used in functions that return a path in sfct_api
    struct path_ext{
        std::filesystem::path p;            // the file path
        std::error_code e;                  // error code
    };

    // used in the get file size function in sfct_api
    struct file_size_ext{
        std::uintmax_t size;                // size of the file in bytes
        std::error_code e;                  // error code
    };

    // used in the copy file functions in sfct_api
    struct copy_file_ext{
        bool rv;                            // return value
        std::error_code e;                  // error code
    };

    // sets the status of a file entry
    enum class file_queue_status{
        file_added,                         
        file_updated,
        file_removed,
        directory_added,
        directory_removed,
        directory_updated,
        other_added,
        other_removed,
        other_updated,
        rename_old,
        rename_new,
        none
    };

    // is a file entry object
    struct file_queue_info{
        std::filesystem::path src,dst,main_src,main_dst;
        std::filesystem::copy_options co;
        std::filesystem::file_status fs_src,fs_dst;
        file_queue_status fqs;
        cs commands;

        bool operator==(const file_queue_info& other) const {
            return src == other.src && dst == other.dst;
        }
    };

    // used in the remove file functions in sfct_api
    struct remove_file_ext{
        enum class remove_file_status{
            removal_success,
            error_code_present,
            invalid_entry
        };
        
        bool rv;                                // return value
        std::error_code e;                      // error code
        remove_file_status s;                   // status of the operation
    };

    // used in the copy symlink function in sfct_api
    struct copy_sym_ext{
        std::filesystem::path target;           // the target link
        std::error_code e;                      // error code
    };

    // used in the is functions in sfct_api
    struct is_entry_ext{
        std::error_code e;                      // error code
        bool rv;                                // return value
    };

    // used in the last_write functions in sfct_api
    struct last_write_ext{
        std::filesystem::file_time_type t;      // the time of last write
        std::error_code e;                      // error code
    };

    // used in get_file_status function in sfct_api
    struct file_status_ext{
        std::filesystem::file_status s;         // file status (the type of file)
        std::error_code e;                      // error code
    };

    // used in the remove_all() function in sfct_api
    struct remove_all_ext{
        enum class remove_all_status{
            removal_success,
            exception_thrown,
            error_code_present,
            invalid_directory
        };
        
        std::uintmax_t files_removed;           // number of files removed from the system
        std::error_code e;                      // error code
        remove_all_status s;                    // state/status of the removal operation                 
    };

    // used to check whether a directory has entries or not
    struct is_directory_empty_ext{
        enum class directory_status{
            invalid_directory,
            empty,
            has_entries,
            exception_thrown
        };
        directory_status s;
        bool rv;
    };
}

// used in queue_system.hpp to tell the difference between two file_queue_info objects
// in a set or map
namespace std {
    template<>
    struct hash<application::file_queue_info> {
        std::size_t operator()(const application::file_queue_info& fqi) const noexcept {
            // Compute individual hash values for two members for simplicity here
            // and combine them using a method similar to boost::hash_combine
            std::size_t h1 = std::hash<std::filesystem::path>{}(fqi.src);
            std::size_t h2 = std::hash<std::filesystem::path>{}(fqi.dst);
            return h1 ^ (h2 << 1); // Simple example of combining hashes
        }
    };
}