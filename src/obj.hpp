#pragma once
#include <filesystem>
#include "args.hpp"
#include <functional>


/////////////////////////////////////////////////////////////////
// This header contains common structures that are used throughout the program.
/////////////////////////////////////////////////////////////////


namespace application{
     struct copyto{
        // source directory path to monitor or copy from
        std::filesystem::path source;

        // destination directory path to copy or sync to
        std::filesystem::path destination; 

        // holds the arguments whether to copy or monitor, sync, update ect.         
        cs commands = cs::none; 

        // holds the arguments in std::filesystem::copy_options format                    
        std::filesystem::copy_options co = std::filesystem::copy_options::none;          
    };

    inline bool copyto_equal(const copyto& a, const copyto& b){
        // Define what makes two copyto objects equal
        return a.source == b.source && a.destination == b.destination && a.commands == b.commands && a.co == b.co;
    }

    inline bool copyto_comparison(const copyto& a, const copyto& b){
        // Define ordering for copyto objects
        if (a.source != b.source) return a.source < b.source;
        if (a.destination != b.destination) return a.destination < b.destination;
        if (a.commands != b.commands) return a.commands < b.commands;
        return a.co < b.co;
    }

    struct directory_info{
        std::uintmax_t TotalSize;
        std::uintmax_t FileCount;
        double_t AvgFileSize;
    };

    struct path_ext{
        std::filesystem::path p;
        std::error_code e;
    };

    struct file_size_ext{
        std::uintmax_t size;
        std::error_code e;
    };

    struct copy_file_ext{
        // returned value from the function std::filesystem::copy_file()
        bool rv;

        // error code from the function std::filesystem::copy_file() 
        std::error_code e;
    };

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
        none
    };

    struct file_queue_info{
        std::filesystem::path src,dst;
        std::filesystem::copy_options co;
        std::filesystem::file_status fs_src,fs_dst;
        file_queue_status fqs;
        cs commands;

        bool operator==(const file_queue_info& other) const {
            // Define equality based on the members that uniquely identify the object
            // This is a simple example; adjust according to your needs
            return src == other.src && dst == other.dst;
        }
    };

    struct remove_file_ext{
        bool rv;
        std::uintmax_t files_removed;
        std::error_code e;
    };
}

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