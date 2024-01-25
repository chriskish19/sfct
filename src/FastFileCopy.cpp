#include "FastFileCopy.hpp"

#if WINDOWS_BUILD
application::FastFileCopy::FastFileCopy(std::shared_ptr<std::vector<copyto>> dirs)
:m_dirs(dirs)
{
    if(!dirs){
        logger log(App_MESSAGE("nullptr"),Error::FATAL);
        log.to_console();
        log.to_log_file();
        throw std::runtime_error("");
    }


}

void application::FastFileCopy::copy()
{
    for(const auto& dir:*m_dirs){
        directory_info counts = GetDI(dir);
        
        // less than 1mb 
        if(counts.AvgFileSize < (1024 * 1024)){
            // use std::filesystem::copy its faster than mapping
            benchmark speed;
            speed.start_clock();
            std::filesystem::copy(dir.source,dir.destination,dir.co);
            speed.end_clock();

            double rate = speed.speed(counts.TotalSize);

            m_MessageStream.SetMessage(App_MESSAGE("Speed in MB/s: ") + TOSTRING(rate));
            m_MessageStream.ReleaseBuffer();
        }
        else{
            // use Windows::FastCopy
            if((dir.commands & cs::recursive) != cs::none){
                std::uintmax_t totalsize{};
                benchmark speed;
                speed.start_clock();
                totalsize = recursive(dir);
                speed.end_clock();

                double rate = speed.speed(totalsize);

                m_MessageStream.SetMessage(App_MESSAGE("Speed in MB/s: ") + TOSTRING(rate));
                m_MessageStream.ReleaseBuffer();
            }
            else if((dir.commands & cs::single) != cs::none){
                std::uintmax_t totalsize{};
                benchmark speed;
                speed.start_clock();
                totalsize = single(dir);
                speed.end_clock();

                double rate = speed.speed(totalsize);

                m_MessageStream.SetMessage(App_MESSAGE("Speed in MB/s: ") + TOSTRING(rate));
                m_MessageStream.ReleaseBuffer();
            }
        }


        
    }
}

std::uintmax_t application::FastFileCopy::recursive(const copyto& dir)
{
    std::uintmax_t totalsize{};
    for(const auto& entry:std::filesystem::recursive_directory_iterator(dir.source)){
        totalsize += entry.file_size();
        const auto& path = entry.path();
        auto relativePath = std::filesystem::relative(path, dir.source);
        if (entry.is_directory()) {
            std::filesystem::create_directories(dir.destination / relativePath);
        } else if (entry.is_regular_file() && entry.file_size() != 0) {
            paths* _paths = new paths(path,dir.destination / relativePath);
            m_pPaths.push_back(_paths);
            m_workers.do_work(&Windows::FastCopy,_paths->m_src.c_str(),_paths->m_dst.c_str());
            if(m_workers.join_one()){
                paths* p_path = m_pPaths.front();
                if(p_path) delete p_path;
                m_pPaths.erase(m_pPaths.begin());
            }
        }
    }
    m_workers.join_all();

    // cleanup
    for(const auto path:m_pPaths){
        if(path) delete path;
    }
    m_pPaths.clear();

    return totalsize;
}

std::uintmax_t application::FastFileCopy::single(const copyto &dir)
{
    std::uintmax_t totalsize{};
    for(const auto& entry:std::filesystem::directory_iterator(dir.source)){
        totalsize += entry.file_size();
        const auto& path = entry.path();
        auto relativePath = std::filesystem::relative(path, dir.source);
        if(entry.is_directory()){
            std::filesystem::create_directory(dir.destination/relativePath);
        }
        else if (entry.is_regular_file() && entry.file_size() != 0) {
            paths* _paths = new paths(path,dir.destination / relativePath);
            m_pPaths.push_back(_paths);
            m_workers.do_work(&Windows::FastCopy,_paths->m_src.c_str(),_paths->m_dst.c_str());
            if(m_workers.join_one()){
                paths* p_path = m_pPaths.front();
                if(p_path) delete p_path;
                m_pPaths.erase(m_pPaths.begin());
            }
        }
    }
    m_workers.join_all();

    // cleanup
    for(const auto path:m_pPaths){
        if(path) delete path;
    }
    m_pPaths.clear();
    
    return totalsize;
}

#endif