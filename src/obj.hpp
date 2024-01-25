#pragma once
#include <filesystem>
#include "args.hpp"

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
        double AvgFileSize;
    };

    
    struct paths{
        paths(const std::filesystem::path& src,const std::filesystem::path& dst)
        :m_src(src),
        m_dst(dst){}

        const std::filesystem::path m_src;
        const std::filesystem::path m_dst;
    };
}