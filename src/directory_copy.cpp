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
    std::thread q_sys_thread([this](){
    exceptions(
        &application::queue_system<application::file_queue_info>::process, 
        &m_queue_processor
    );
    });

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

                    m_queue_processor.add_to_queue(_file_info);
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

                m_queue_processor.add_to_queue(_file_info);
            }
            


        }
        
        m_queue_processor.m_ready_to_process = true;
        m_queue_processor.m_local_thread_cv.notify_one();
    }

    m_queue_processor.exit();
    if(q_sys_thread.joinable()){
        q_sys_thread.detach();
    }
}
