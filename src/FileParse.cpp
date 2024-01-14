#include "FileParse.hpp"

application::FileParse::FileParse(const std::filesystem::path& path):m_FilePath(path){
    // check if the file exists
    m_FileExists = std::filesystem::exists(m_FilePath);
}

void application::FileParse::ExtractData(){
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

    ParseSyntax();

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

void application::FileParse::CheckData(){
    if(!m_DataExtracted){
        logger log(App_MESSAGE("Data has not been extracted, you need to call ExtractData() before calling CheckData()"),Error::DEBUG);
        log.to_console();
        log.to_log_file();
        return;
    }
    
    CheckDirectories();
}

void application::FileParse::ParseSyntax()
{
    copyto directory;
    std::string line;
    while(std::getline(m_File,line)){
        std::istringstream lineStream(line);
        std::string token;
        lineStream >> token;
        auto found_token = global_tokenizer.Find(token);
        if(found_token.has_value()){
            switch(found_token.value()){
                case cs::copy:{
                    directory.commands.insert(cs::copy);
                    while(lineStream >> token){
                        found_token = global_tokenizer.Find(token);
                        if(found_token.has_value()){
                            switch(found_token.value()){
                                case cs::recursive:{
                                    directory.commands.insert(cs::recursive);
                                    break;
                                }
                                case cs::update:{
                                    directory.commands.insert(cs::update);
                                    break;
                                }
                                case cs::overwrite:{
                                    if(directory.commands.find(cs::update)==directory.commands.end()){
                                        directory.commands.insert(cs::overwrite);
                                    }
                                    break;
                                }
                                case cs::single:{
                                    if(directory.commands.find(cs::recursive)==directory.commands.end()){
                                        directory.commands.insert(cs::single);
                                    }
                                    break;
                                }
                                default:{
                                    break;
                                }
                            }
                        }
                    }
                    bool end{false};
                    while(std::getline(m_File,line) && !end){
                        lineStream = std::istringstream(line);
                        lineStream >> token;
                        found_token = global_tokenizer.Find(token);
                        if(found_token.has_value()){
                            switch(found_token.value()){
                                case cs::open_brace:{
                                    continue;
                                    break;
                                }
                                case cs::src:{
                                    std::getline(lineStream,line);
                                    size_t begin_pos = line.find_first_not_of(" ");
                                    size_t end_pos = line.find_last_of(';');
                                    std::string src_dir(line.begin()+begin_pos,line.end()-end_pos);
                                    directory.source = std::filesystem::path(src_dir);
                                    break;
                                }
                                case cs::dst:{
                                    std::getline(lineStream,line);
                                    size_t begin_pos = line.find_first_not_of(" ");
                                    size_t end_pos = line.find_last_of(';');
                                    std::string dst_dir(line.begin()+begin_pos,line.end()-end_pos);
                                    directory.destination = std::filesystem::path(dst_dir);
                                    break;
                                }
                                case cs::close_brace:{
                                    end = true;
                                    m_Data->push_back(directory);
                                    directory = {};
                                    break;
                                }
                                default:{
                                    break;
                                }
                                
                            }
                        }
                    }
                    break;
                }
                case cs::monitor:{
                    directory.commands.insert(cs::monitor);
                    while(lineStream >> token){
                         found_token = global_tokenizer.Find(token);
                         if(found_token.has_value()){
                            switch(found_token.value()){
                                case cs::sync:{
                                    directory.commands.insert(cs::sync);
                                    break;
                                }
                                case cs::sync_add:{
                                    if(directory.commands.find(cs::sync)==directory.commands.end()){
                                        directory.commands.insert(cs::sync_add);
                                    }
                                    break;
                                }
                                default:{
                                    break;
                                }
                            }
                         }
                    }

                    bool end{false};
                    while(std::getline(m_File,line) && !end){
                        lineStream = std::istringstream(line);
                        lineStream >> token;
                        found_token = global_tokenizer.Find(token);
                        if(found_token.has_value()){
                            switch(found_token.value()){
                                case cs::open_brace:{
                                    continue;
                                    break;
                                }
                                case cs::src:{
                                    std::getline(lineStream,line);
                                    size_t begin_pos = line.find_first_not_of(" ");
                                    size_t end_pos = line.find_last_of(';');
                                    std::string src_dir(line.begin()+begin_pos,line.end()-end_pos);
                                    directory.source = std::filesystem::path(src_dir);
                                    break;
                                }
                                case cs::dst:{
                                    std::getline(lineStream,line);
                                    size_t begin_pos = line.find_first_not_of(" ");
                                    size_t end_pos = line.find_last_of(';');
                                    std::string dst_dir(line.begin()+begin_pos,line.end()-end_pos);
                                    directory.destination = std::filesystem::path(dst_dir);
                                    break;
                                }
                                case cs::close_brace:{
                                    end = true;
                                    m_Data->push_back(directory);
                                    directory = {};
                                    break;
                                }
                                default:{
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
                case cs::fast_copy:{
                    directory.commands.insert(cs::fast_copy);
                    while(lineStream >> token){
                        found_token = global_tokenizer.Find(token);
                        if(found_token.has_value()){
                            switch(found_token.value()){
                                case cs::recursive:{
                                    directory.commands.insert(cs::recursive);
                                    break;
                                }
                                case cs::update:{
                                    directory.commands.insert(cs::update);
                                    break;
                                }
                                case cs::overwrite:{
                                    if(directory.commands.find(cs::update)==directory.commands.end()){
                                        directory.commands.insert(cs::overwrite);
                                    }
                                    break;
                                }
                                case cs::single:{
                                    if(directory.commands.find(cs::recursive)==directory.commands.end()){
                                        directory.commands.insert(cs::single);
                                    }
                                    break;
                                }
                                default:{
                                    break;
                                }
                            }
                        }
                    }
                    bool end{false};
                    while(std::getline(m_File,line) && !end){
                        lineStream = std::istringstream(line);
                        lineStream >> token;
                        found_token = global_tokenizer.Find(token);
                        if(found_token.has_value()){
                            switch(found_token.value()){
                                case cs::open_brace:{
                                    continue;
                                    break;
                                }
                                case cs::src:{
                                    std::getline(lineStream,line);
                                    size_t begin_pos = line.find_first_not_of(" ");
                                    size_t end_pos = line.find_last_of(';');
                                    std::string src_dir(line.begin()+begin_pos,line.end()-end_pos);
                                    directory.source = std::filesystem::path(src_dir);
                                    break;
                                }
                                case cs::dst:{
                                    std::getline(lineStream,line);
                                    size_t begin_pos = line.find_first_not_of(" ");
                                    size_t end_pos = line.find_last_of(';');
                                    std::string dst_dir(line.begin()+begin_pos,line.end()-end_pos);
                                    directory.destination = std::filesystem::path(dst_dir);
                                    break;
                                }
                                case cs::close_brace:{
                                    end = true;
                                    m_Data->push_back(directory);
                                    directory = {};
                                    break;
                                }
                                default:{
                                    break;
                                }
                                
                            }
                        }
                    }

                    break;
                }
                default:{
                    break;
                }
            }
        }
    }   
}




void application::FileParse::CheckDirectories(){
    for(auto it{m_Data->begin()};it!=m_Data->end();){
        if(!std::filesystem::exists(it->source) && !std::filesystem::exists(it->destination)){
            logger log(App_MESSAGE("Invalid entry"),Error::WARNING,it->source);
            log.to_console();
            log.to_log_file();
            m_Data->erase(it);
        }
        else{
            it++;
        }
    }

    if(m_Data->empty()){
        logger log(App_MESSAGE("No Valid directories"),Error::FATAL);
        log.to_console();
        log.to_log_file();
        throw std::runtime_error("");
    }
}


