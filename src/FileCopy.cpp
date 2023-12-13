#include "FileCopy.hpp"

void application::FileCopy::copy_file(){
    while(application::MT::running){
         // this guards multiple threads using the same FileCopy object
        // each thread needs their own copy of FileCopy object to work 
        // concurrently
        std::lock_guard<std::mutex> local_lock(m_copy_file_mtx);
        
        // a bit inefficient currently 
        // TODO: optimize in the future
        // too much class object coping going on
        copyto file_directory = next_file();

        // signals no more files left to copy
        if(application::MT::no_files_left){
            break;
        }

        // set the current directory to be used for outputting copy messages to the console
        m_CurrentDirectory = file_directory;

        // copy the file 
        if(!std::filesystem::copy_file(file_directory.fs_source,file_directory.fs_destination,m_co)){
            std::wstring filepath{file_directory.fs_source};
            logger log(L"Failed to copy file: " + filepath,Error::WARNING,App_LOCATION);
            log.to_console();
            log.to_log_file();
        }
    }
}

application::FileCopy::FileCopy(const std::shared_ptr<FileCopyState> pState):m_pState(pState){
    // check that m_pState is valid
    // error handling
    if(!m_pState){
        logger log(L"m_pState is nullptr",Error::FATAL,App_LOCATION);
        log.to_console();
        log.to_log_file();
        throw std::runtime_error("dereferencing a nullptr");
    }
    
    // copy copy options
    m_co = m_pState->m_CO;
}
    


const application::copyto application::FileCopy::next_file(){
    // prevents concurent access to the shared resources in 
    // the FileCopyState class which are read and possibly modified in 
    // this function, the lock will unlock automatically when the function 
    // goes out of scope
    std::lock_guard<std::mutex> local_lock(application::MT::m_next_file_mtx);
    
    // the directories object to return which contains the paths to the next file
    // to be copied
    copyto file_directory;

    if((m_co & std::filesystem::copy_options::recursive) != std::filesystem::copy_options::none){
        
        // initialize the recursive iterator
        if(!m_pState->m_rdit_init){
            m_pState->m_rdit = std::filesystem::recursive_directory_iterator(m_pState->m_pDirectories->at(m_pState->m_Directories_index).fs_source);

            m_pState->m_rdit_init = true;
        }

        // check for directory entries to visit
        if(m_pState->m_rdit != m_pState->m_rdendit){
            // Check for cycles and skip if already visited
            if (!m_pState->m_visited_directories.insert(m_pState->m_rdit->path()).second) {
                // if a cycle is found advance to the next folder or file
                m_pState->m_rdit++;
            }

            // set source entry, useful for file information
            file_directory.src_entry = *(m_pState->m_rdit);

            // set fs_source path
            file_directory.fs_source = file_directory.src_entry.path();

            // Get the relative path of the source file with respect to its parent directory
            std::filesystem::path relative_path = file_directory.fs_source.lexically_relative(file_directory.fs_source.parent_path());

            // set fs_destination
            file_directory.fs_destination = m_pState->m_pDirectories->at(m_pState->m_Directories_index).fs_destination / relative_path;

            // create the directory tree
            if(!std::filesystem::exists(file_directory.fs_destination.parent_path())){
                std::filesystem::create_directories(file_directory.fs_destination.parent_path());
            }

            // advance to the next entry
            // double check were not at the end since we could be
            // if there was a cycle
            if(m_pState->m_rdit != m_pState->m_rdendit){
                m_pState->m_rdit++;
            }

            return file_directory;
        }
        else if(m_pState->m_Directories_index < m_pState->m_pDirectories->size()){
            // can clear the visted directories now
            m_pState->m_visited_directories.clear();
            
            // set directory iterator to a new directory and advance the directories vector index
            m_pState->m_rdit = std::filesystem::recursive_directory_iterator(m_pState->m_pDirectories->at(++m_pState->m_Directories_index).fs_source);

            // set source entry, useful for file information
            file_directory.src_entry = *(m_pState->m_rdit);

            // set fs_source path
            file_directory.fs_source = file_directory.src_entry.path();

            // Get the relative path of the source file with respect to its parent directory
            std::filesystem::path relative_path = file_directory.fs_source.lexically_relative(file_directory.fs_source.parent_path());

            // set fs_destination
            file_directory.fs_destination = m_pState->m_pDirectories->at(m_pState->m_Directories_index).fs_destination / relative_path;

            // create the directory tree
            if(!std::filesystem::exists(file_directory.fs_destination.parent_path())){
                std::filesystem::create_directories(file_directory.fs_destination.parent_path());
            }

            return file_directory;
        }
        else{
            // exit no more directories to copy 
            application::MT::running = false;
        }
    }
    else{
        if(!m_pState->m_dit_init){
            m_pState->m_dit = std::filesystem::directory_iterator(m_pState->m_pDirectories->at(m_pState->m_Directories_index).fs_source);

            m_pState->m_dit_init = true; 
        }

        if(m_pState->m_dit != m_pState->m_ditend){
            // set source entry, useful for file information
            file_directory.src_entry = *(m_pState->m_dit);

            // set fs_source path
            file_directory.fs_source = file_directory.src_entry.path();

            // set fs_destination
            file_directory.fs_destination = m_pState->m_pDirectories->at(m_pState->m_Directories_index).fs_destination;

            // adavnce to the next entry
            m_pState->m_dit++;

            return file_directory;
        }
        else if(m_pState->m_Directories_index < m_pState->m_pDirectories->size()){
            // set directory iterator to a new directory and advance the directories vector index
            m_pState->m_dit = std::filesystem::directory_iterator(m_pState->m_pDirectories->at(++m_pState->m_Directories_index).fs_source);

            // set source entry, useful for file information
            file_directory.src_entry = *(m_pState->m_dit);

            // set fs_source path
            file_directory.fs_source = file_directory.src_entry.path();

            // set fs_destination
            file_directory.fs_destination = m_pState->m_pDirectories->at(m_pState->m_Directories_index).fs_destination;

            return file_directory;
        }
        else{
            // exit no more directories to copy 
            application::MT::running = false;
        }
    }
    
    // no more files left to return
    application::MT::no_files_left = true;
    return file_directory;
}



////////////////////////////////////////
/* FileCopyState function definitions */
////////////////////////////////////////

application::FileCopyState::FileCopyState(const std::shared_ptr<std::vector<copyto>> pDirectories,const std::filesystem::copy_options co):m_pDirectories(pDirectories),m_CO(co){
    //error handling here
    if(!m_pDirectories){
        logger log(L"m_pDirectories is nullptr",Error::FATAL,App_LOCATION);
        log.to_console();
        log.to_log_file();
        throw std::runtime_error("dereferencing a nullptr");
    }

    if(m_pDirectories->empty()){
        logger log(L"m_pDirectories vector is empty",Error::FATAL,App_LOCATION);
        log.to_console();
        log.to_log_file();
        throw std::runtime_error("empty vector");
    }
}