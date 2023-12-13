#include "DirectorySignal.hpp"

#ifdef WINDOWS_BUILD
application::DirectorySignal::DirectorySignal(std::filesystem::path path_to_watch){
    // open the directory
    m_hDirectory = CreateFile(
                    path_to_watch.c_str(),
                    FILE_LIST_DIRECTORY,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_BACKUP_SEMANTICS,
                    NULL);

    if(m_hDirectory == INVALID_HANDLE_VALUE){
        logger log(Error::WARNING,App_LOCATION);
        log.to_console();
        log.to_log_file();
        log.to_output();
    }
}

bool application::DirectorySignal::signal(){
    
}

application::DirectorySignal::~DirectorySignal(){
    CloseHandle(m_hDirectory);
}



#endif