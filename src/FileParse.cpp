#include "FileParse.hpp"

application::FileParse::FileParse(const std::filesystem::path& path):m_FilePath(path){
    // check if the file exists
    m_FileExists = std::filesystem::exists(m_FilePath);
}

void application::FileParse::ExtractData(const std::string& keyword){
    if(!m_File.is_open()){
        logger log(L"You need to Open the file before extracting the data, ExtractData will return to the caller without executing further",Error::DEBUG,App_LOCATION);
        log.to_console();
        log.to_log_file();
        return;
    }

    // if m_Data has data in it, it means the data has already been extracted
    if(!m_Data->empty() || m_DataExtracted){
        logger log(L"Data has already been extracted, returning to function caller",Error::INFO,App_LOCATION);
        log.to_console();
        log.to_log_file();
        return;
    }

    std::string line;
    while(std::getline(m_File,line)){
        std::istringstream lineStream(line);
        std::string prefix;
        lineStream >> prefix;

        if(prefix == keyword){
            copyto directory;
            lineStream >> directory.source >> directory.destination;
            m_Data->push_back(directory);
        }
    }

    // this function should only be called once per object unless a new valid path is set with 
    // SetFilePath() 
    m_DataExtracted = true;
}

void application::FileParse::OpenFile(){
    // the file does not exist
    // dont execute the function instead return to the caller
    if(!m_FileExists){
        logger log(L"File does not exist, OpenFile function will return to the caller without executing further, use SetFilePath() to set a new valid path",Error::DEBUG,App_LOCATION);
        log.to_console();
        log.to_log_file();
        return;
    }
    
    // open the file for reading
    m_File.open(m_FilePath,std::ios::in);

    if(!m_File.is_open()){
        logger log(L"Failed to open file for reading",Error::WARNING,App_LOCATION);
        log.to_console();
        log.to_log_file();
    }
    else{
        logger log(L"Succesfully opened the file",Error::INFO,App_LOCATION);
        log.to_console();
        log.to_log_file();
    }
}

application::FileParse::FileParse(const std::string& filename){
    // set m_FilePath to the current working directory
    m_FilePath = std::filesystem::current_path();

    // append the file name to m_FilePath
    m_FilePath/=filename;

    // check if the file exists
    m_FileExists = std::filesystem::exists(m_FilePath);
}

void application::FileParse::SetFilePath(const std::filesystem::path& new_path){
    // check if the new_path is valid
    bool path_exists = std::filesystem::exists(new_path);

    if(!path_exists){
        logger log(L"New path is not valid",Error::DEBUG,App_LOCATION);
        log.to_console();
        log.to_log_file();
    }
    else{
        // the path is valid
        // set the member variables now
        m_FileExists = path_exists;
        m_FilePath = new_path;

        // reset 
        m_DataExtracted = false;

        // clear the data
        m_Data->clear();

        // close the file
        // if close fails an exception is thrown automatically by fstream
        m_File.close();
    }

    
}

void application::FileParse::CheckData(DataType t){
    if(!m_DataExtracted){
        logger log(L"Data has not been extracted, you need to call ExtractData() before calling CheckData()",Error::DEBUG,App_LOCATION);
        log.to_console();
        log.to_log_file();
        return;
    }
    
    
    switch(t){
        case DataType::Directory:{
            CheckDirectories();
            break;
        }
        case DataType::Text:{
            CheckText();
            break;
        }
        default:{return;}
    }
    return;
}

void application::FileParse::CheckText(){
    return; // blank for now
}


void application::FileParse::CheckDirectories(){
    // check that the directories are valid
    // if the source entry is not valid mark the entry to be removed
    // if the destination does not exist it will be created
    for(size_t i{};i<m_Data->size();){
        if(!std::filesystem::exists(m_Data->at(i).source)){
            std::puts("The copy directory is invalid, its entry will not be used");
            std::cout << m_Data->at(i).source << "\n";

            // erase the entry
            // Do not increment i, as the next element has shifted to the current position
            m_Data->erase(m_Data->begin() + i);
        }
        else{
            // Only increment i if an element was not erased
            i++;
        }
    }
    
    // check that the destination exists and if it does not, attempt to create it
    for(size_t i{};i<m_Data->size();i++){
        if(!std::filesystem::exists(m_Data->at(i).destination)){
            if(!std::filesystem::create_directories(m_Data->at(i).destination)){
                std::puts("Error: failed to create directories, destination is an invalid path");
                std::cout << m_Data->at(i).destination << "\n";

                // mark the directory as invalid
                m_Data->at(i).destination = "INVALID";
                m_Data->at(i).source = "INVALID";
            }
        }
    }

    // erase the invalid directories
    for(size_t i{};i<m_Data->size();){
        if(m_Data->at(i).destination == "INVALID"){
            m_Data->erase(m_Data->begin()+i);
        }
        else{
            i++;
        }
    }

    // check that m_Data has entries
    if(m_Data->empty()){
        logger log(L"No valid directory entries in file",Error::FATAL,m_FilePath,App_LOCATION);
        log.to_console();
        log.to_log_file();
        throw std::runtime_error("No valid directories found");
    }


    // initialize fs_source and fs_destination in the m_Data vector
    // using the std::string's source and destination
    // std::filesystem works best with its native path type
    for(size_t i{};i<m_Data->size();i++){
        std::filesystem::path src_init(m_Data->at(i).source);
        std::filesystem::path dest_init(m_Data->at(i).destination);
        m_Data->at(i).fs_source = src_init;
        m_Data->at(i).fs_destination = dest_init;
    }
}