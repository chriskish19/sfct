#include "FileParse.hpp"

application::FileParse::FileParse(const std::filesystem::path& path):m_FilePath(path){
    // check if the file exists
    m_FileExists = std::filesystem::exists(m_FilePath);
}

void application::FileParse::ExtractData(const std::string& keyword){
    if(!m_File.is_open()){
        logger log(App_MESSAGE("You need to Open the file before extracting the data, ExtractData will return to the caller without executing further"),Error::DEBUG);
        log.to_console();
        log.to_log_file();
        return;
    }

    // if m_Data has data in it, it means the data has already been extracted
    if(!m_Data->empty() || m_DataExtracted){
        logger log(App_MESSAGE("Data has already been extracted, returning to function caller"),Error::INFO);
        log.to_console();
        log.to_log_file();
        return;
    }

    copyto directory;
    std::string line;
    bool src_set{false},dst_set{false};

    while(std::getline(m_File,line)){
        std::istringstream lineStream(line);
        std::string token;
        lineStream >> token;
        if(token == keyword){
            continue;
        }
        else if(token == "{"){
            continue;
        }
        else if(token == "src"){
            std::getline(lineStream,line);
            
            // remove leading whitespace
            size_t begin_pos = line.find_first_not_of(" ");
            std::string new_line(line.begin()+begin_pos,line.end());

            size_t last_pos = new_line.find_last_of(';');
            if(last_pos!=std::string::npos){
                new_line.erase(new_line.begin()+last_pos);
                directory.source = new_line;
                src_set = true;
            }
        }
        else if(token == "dst"){
            std::getline(lineStream,line);
            
            // remove leading whitespace
            size_t begin_pos = line.find_first_not_of(" ");
            std::string new_line(line.begin()+begin_pos,line.end());

            size_t last_pos = new_line.find_last_of(';');
            if(last_pos!=std::string::npos){
                new_line.erase(new_line.begin()+last_pos);
                directory.destination = new_line;
                dst_set = true;
            }
        }
        else if(token == "}"){
            continue;
        }

        if(src_set && dst_set){
            m_Data->push_back(directory);
            directory = {};
            src_set = false;
            dst_set = false;
        }
    }


    // this function should only be called once per object unless a new valid path is set with 
    // SetFilePath() 
    m_DataExtracted = true;
}

bool application::FileParse::OpenFile(){
    // the file does not exist
    // dont execute the function instead return false to the caller
    if(!m_FileExists){
        logger log(App_MESSAGE("File does not exist, use SetFilePath() to set a new valid path"),Error::DEBUG);
        log.to_console();
        log.to_log_file();
        return false;
    }
    
    // open the file for reading
    m_File.open(m_FilePath,std::ios::in);

    if(!m_File.is_open()){
        logger log(App_MESSAGE("Failed to open file for reading"),Error::FATAL);
        log.to_console();
        log.to_log_file();
        throw std::runtime_error("std::fstream fail");
    }
    else{
        logger log(App_MESSAGE("Succesfully opened sfct_list.txt"),Error::INFO);
        log.to_console();
        log.to_log_file();
    }

    return true;
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
        logger log(App_MESSAGE("New path is not valid"),Error::DEBUG);
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
        logger log(App_MESSAGE("Data has not been extracted, you need to call ExtractData() before calling CheckData()"),Error::DEBUG);
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
            logger log(App_MESSAGE("entry is invalid and will not be used"),Error::WARNING,std::filesystem::path(m_Data->at(i).source));
            log.to_console();
            log.to_log_file();
            log.to_output();

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
                logger log(App_MESSAGE("Error: failed to create directories, destination is an invalid path"),Error::WARNING,std::filesystem::path(m_Data->at(i).destination));
                log.to_console();
                log.to_log_file();
                log.to_output();
                
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
        logger log(App_MESSAGE("No valid directory entries in file"),Error::FATAL,m_FilePath);
        log.to_console();
        log.to_log_file();
        throw std::runtime_error("No valid directories found, program will now exit");
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
