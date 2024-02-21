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
        auto di = sfct_api::get_directory_info(dir);
        if(di.has_value()){
            STDOUT << App_MESSAGE("Copying directory: ") << dir.source << App_MESSAGE(" to: ") << dir.destination << "\n";
            STDOUT << App_MESSAGE("Total size in bytes: ") << di.value().TotalSize << "\n";
            STDOUT << App_MESSAGE("Total number of files: ") << di.value().FileCount << "\n";
        }

        benchmark test;
        test.start_clock();
        sfct_api::copy_entry(dir.source,dir.destination,dir.co);
        test.end_clock();

        double_t rate = test.speed(di.value().TotalSize);

        STDOUT << App_MESSAGE("Transfer speed in MB/s: ") << rate << "\n";
    }
}

void application::directory_copy::copy()
{

    for(const auto& dir:*m_dirs){
        
        auto di = sfct_api::get_directory_info(dir);
        if(di.has_value()){
            STDOUT << App_MESSAGE("Copying directory: ") << dir.source << App_MESSAGE(" to: ") << dir.destination << "\n";
            STDOUT << App_MESSAGE("Total size in bytes: ") << di.value().TotalSize << "\n";
            STDOUT << App_MESSAGE("Total number of files: ") << di.value().FileCount << "\n";
        }


        benchmark test;
        test.start_clock();
        if(sfct_api::recursive_flag_check(dir.commands)){
            
            STRING prev_entry_path;

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

                    sfct_api::output_entry_to_console(entry,prev_entry_path.length());
                    prev_entry_path = STRING(entry.path());

                    m_queue_processor.public_process_entry(_file_info);
                }
            }


        }
        else{
            
            STRING prev_entry_path;

            for(const auto& entry:std::filesystem::directory_iterator(dir.source)){
                file_queue_info _file_info;
                _file_info.co = dir.co;
                _file_info.dst = dir.destination;
                _file_info.fqs = file_queue_status::file_added;
                _file_info.fs_dst = std::filesystem::status(dir.destination);
                _file_info.fs_src = std::filesystem::status(entry.path());
                _file_info.src = entry.path();

                sfct_api::output_entry_to_console(entry,prev_entry_path.length());
                prev_entry_path = STRING(entry.path());

                m_queue_processor.public_process_entry(_file_info);
            }
        
        }
        test.end_clock();

        double_t rate = test.speed(di.value().TotalSize);

        STDOUT << "\n";
        STDOUT << App_MESSAGE("Transfer speed in MB/s: ") << rate << "\n";
    }
    


}


