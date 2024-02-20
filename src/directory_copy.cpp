#include "directory_copy.hpp"

application::directory_copy::directory_copy(std::shared_ptr<std::vector<copyto>> dirs)
:m_dirs(dirs)
{
    if(!dirs){
        logger log(App_MESSAGE("nullptr"),Error::FATAL);
        log.to_console();
        log.to_log_file();
        throw std::runtime_error("");
    }
}

void application::directory_copy::fast_copy()
{
    for(const auto& dir:*m_dirs){
        sfct_api::copy_entry(dir.source,dir.destination,dir.co);
    }
}

void application::directory_copy::copy()
{

    for(const auto& dir:*m_dirs){
        
        if(sfct_api::recursive_flag_check(dir.commands)){
            
            for(const auto& entry:std::filesystem::recursive_directory_iterator(dir.source)){
                std::optional<std::filesystem::path> succeeded = sfct_api::create_file_relative_path(entry.path(),dir.destination,dir.source,false);
                if(succeeded.has_value()){
                    file_queue_info _file_info;
                    _file_info.co = dir.co;
                    _file_info.dst = succeeded.value();
                    _file_info.fqs = file_queue_status::file_added;
                    _file_info.fs_dst = std::filesystem::status(succeeded.value());
                    _file_info.fs_src = std::filesystem::status(entry.path());
                    _file_info.src = entry.path();

                    m_queue_processor.process_entry(_file_info);
                }
            }



        }
        else{

            for(const auto& entry:std::filesystem::directory_iterator(dir.source)){
                file_queue_info _file_info;
                _file_info.co = dir.co;
                _file_info.dst = dir.destination;
                _file_info.fqs = file_queue_status::file_added;
                _file_info.fs_dst = std::filesystem::status(dir.destination);
                _file_info.fs_src = std::filesystem::status(entry.path());
                _file_info.src = entry.path();

                m_queue_processor.process_entry(_file_info);
            }
        
        }
        
    }
}
