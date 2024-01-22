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
        if((dir.commands & cs::recursive) != cs::none){
            std::filesystem::copy(dir.source,dir.destination,dir.co);
        }
        else if((dir.commands & cs::single) != cs::none){
            std::filesystem::copy(dir.source,dir.destination,dir.co);
        }
    }
}

void application::FastFileCopy::recursive(const copyto& dir)
{
    for(const auto& entry:std::filesystem::recursive_directory_iterator(dir.source)){
        const auto& path = entry.path();
        auto relativePath = std::filesystem::relative(path, dir.source);
        if (entry.is_directory()) {
            std::filesystem::create_directories(dir.destination / relativePath);
        } else if (entry.is_regular_file()) {
            std::filesystem::path dest_path(dir.destination / relativePath);
            Windows::FastCopy(path.c_str(), dest_path.c_str());
        }
    }
}

void application::FastFileCopy::single(const copyto &dir)
{
    for(const auto& entry:std::filesystem::directory_iterator(dir.source)){
        const auto& path = entry.path();
        auto relativePath = std::filesystem::relative(path, dir.source);
        if(entry.is_directory()){
            std::filesystem::create_directory(dir.destination/relativePath);
        }
        else if(entry.is_regular_file()){
            Windows::FastCopy(path.c_str(),dir.destination.c_str());
        }
    }
}

#endif