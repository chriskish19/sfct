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
    std::string line;
    while(std::getline(m_File,line)){
        std::istringstream lineStream(line);
        std::string token;
        lineStream >> token;
        auto found_token = global_tokenizer.Find(token);
        if(found_token.has_value()){
            switch(found_token.value()){
                case cs::copy:{
                    copyto directory{};
                    directory.commands |= cs::copy;
                    directory.commands |= ParseCopyArgs(lineStream);
                    directory.co = GetCopyOptions(directory.commands);
                    ParseDirs(directory);
                    break;
                }
                case cs::monitor:{
                    copyto directory{};
                    directory.commands |= cs::monitor;
                    directory.commands |= ParseMonitorArgs(lineStream);
                    directory.co = GetCopyOptions(directory.commands);
                    ParseDirs(directory);
                    break;
                }
                case cs::fast_copy:{
                    copyto directory{};
                    directory.commands |= cs::fast_copy;
                    directory.commands |= ParseCopyArgs(lineStream);
                    directory.co = GetCopyOptions(directory.commands);
                    ParseDirs(directory);
                    break;
                }
                case cs::benchmark:{
                    copyto directory{};
                    directory.commands |= cs::benchmark;
                    directory.commands |= ParseBenchArgs(lineStream);
                    directory.co |= std::filesystem::copy_options::overwrite_existing;
                    ParseDirs(directory);
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
        if((it->commands & cs::create) != cs::none){

            if(!std::filesystem::exists(it->source)){
                // create the directories for benchmarking
                if(!std::filesystem::create_directories(it->source)){
                    logger log(App_MESSAGE("Failed to create directory"),Error::DEBUG,it->source);
                    log.to_console();
                    log.to_log_file();
                }
            }
            
            if(!std::filesystem::exists(it->destination)){
                if(!std::filesystem::create_directories(it->destination)){
                    logger log(App_MESSAGE("Failed to create directory"),Error::DEBUG,it->destination);
                    log.to_console();
                    log.to_log_file();
                }
            }
            
        }
        
        
        
        if(!std::filesystem::exists(it->source) || 
        !std::filesystem::exists(it->destination) || 
        !ValidCommands(it->commands) ||
        it->source == it->destination){
            logger log(App_MESSAGE("Invalid entry"),Error::WARNING,it->source);
            log.to_console();
            log.to_log_file();
            it = m_Data->erase(it);
        }
        else{
            it++;
        }
    }

    // remove duplicates
    std::sort(m_Data->begin(),m_Data->end(),copyto_comparison);
    auto last = std::unique(m_Data->begin(), m_Data->end(), copyto_equal);
    m_Data->erase(last, m_Data->end());


    if(m_Data->empty()){
        logger log(App_MESSAGE("No Valid directories"),Error::FATAL);
        log.to_console();
        log.to_log_file();
        throw std::runtime_error("");
    }
}

bool application::FileParse::ValidCommands(cs commands)
{
    // regular copy commands
    cs copy_combo1 = cs::copy | cs::recursive | cs::update;
    cs copy_combo2 = cs::copy | cs::recursive | cs::overwrite;
    cs copy_combo3 = cs::copy | cs::single | cs::update;
    cs copy_combo4 = cs::copy | cs::single | cs::overwrite;

    // monitor commands
    cs monitor_combo1 = cs::monitor | cs::recursive | cs::sync | cs::update;
    cs monitor_combo2 = cs::monitor | cs::recursive | cs::sync | cs::overwrite;
    cs monitor_combo3 = cs::monitor | cs::single | cs::sync | cs::update;
    cs monitor_combo4 = cs::monitor | cs::single | cs::sync | cs::overwrite;
    cs monitor_combo5 = cs::monitor | cs::single | cs::sync_add | cs::update;
    cs monitor_combo6 = cs::monitor | cs::single | cs::sync_add | cs::overwrite;
    cs monitor_combo7 = cs::monitor | cs::recursive | cs::sync_add | cs::update;
    cs monitor_combo8 = cs::monitor | cs::recursive | cs::sync_add | cs::overwrite;

    // fast copy commands
    cs fast_copy_combo1 = cs::fast_copy | cs::recursive | cs::update;
    cs fast_copy_combo2 = cs::fast_copy | cs::recursive | cs::overwrite;
    cs fast_copy_combo3 = cs::fast_copy | cs::single | cs::update;
    cs fast_copy_combo4 = cs::fast_copy | cs::single | cs::overwrite;

    // benchmark
    cs benchmark_combo1 = cs::benchmark | cs::create | cs::four_k;
    cs benchmark_combo2 = cs::benchmark | cs::create;
    cs benchmark_combo3 = cs::benchmark | cs::four_k;
    cs benchmark_combo4 = cs::benchmark;
    cs benchmark_combo5 = cs::benchmark | cs::create | cs::fast;
    cs benchmark_combo6 = cs::benchmark | cs::create | cs::four_k | cs::fast;
    cs benchmark_combo7 = cs::benchmark | cs::four_k | cs::fast;
    cs benchmark_combo8 = cs::benchmark | cs::fast;



    return commands == copy_combo1 ||
           commands == copy_combo2 ||
           commands == copy_combo3 ||
           commands == copy_combo4 ||
           commands == monitor_combo1 ||
           commands == monitor_combo2 ||
           commands == monitor_combo3 ||
           commands == monitor_combo4 ||
           commands == monitor_combo5 ||
           commands == monitor_combo6 ||
           commands == monitor_combo7 ||
           commands == monitor_combo8 ||
           commands == fast_copy_combo1 ||
           commands == fast_copy_combo2 ||
           commands == fast_copy_combo3 ||
           commands == fast_copy_combo4 || 
           commands == benchmark_combo1 ||
           commands == benchmark_combo2 ||
           commands == benchmark_combo3 ||
           commands == benchmark_combo4 ||
           commands == benchmark_combo5 ||
           commands == benchmark_combo6 ||
           commands == benchmark_combo7 ||
           commands == benchmark_combo8;
}

application::cs application::FileParse::ParseCopyArgs(std::istringstream &lineStream)
{
    cs commands = cs::none;
    std::string token;
    while(lineStream >> token){
        auto found_token = global_tokenizer.Find(token);
        if(found_token.has_value()){
            switch(found_token.value()){
                case cs::recursive:{
                    if((commands & cs::single) == cs::none){
                        commands |= cs::recursive;
                    }
                    break;
                }
                case cs::update:{
                    if((commands & cs::overwrite) == cs::none){
                        commands |= cs::update;
                    }
                    break;
                }
                case cs::overwrite:{
                    if((commands & cs::update) == cs::none){
                        commands |= cs::overwrite;
                    }
                    break;
                }
                case cs::single:{
                    if((commands & cs::recursive) == cs::none){
                        commands |= cs::single;
                    }
                    break;
                }
                default:{
                    break;
                }
            }
        }
    }
    return commands;
}

void application::FileParse::ParseDirs(copyto &dir)
{
    bool end{false};
    std::string line,token;
    while(std::getline(m_File,line) && !end){
        std::istringstream lineStream(line);
        lineStream >> token;
        auto found_token = global_tokenizer.Find(token);
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
                    if(begin_pos != std::string::npos && end_pos != std::string::npos){
                        std::string src_dir(line.begin() + begin_pos,line.begin() + end_pos);
                        dir.source = std::filesystem::path(src_dir);
                    }
                    else{
                        logger log(App_MESSAGE("Syntax error missing ;"),Error::DEBUG);
                        log.to_console();
                        log.to_log_file();

                        // dir.source is left blank and will be removed when the directories are checked
                    }
                    break;
                }
                case cs::dst:{
                    std::getline(lineStream,line);
                    size_t begin_pos = line.find_first_not_of(" ");
                    size_t end_pos = line.find_last_of(';');
                    if(begin_pos != std::string::npos && end_pos != std::string::npos){
                        std::string dst_dir(line.begin() + begin_pos,line.begin() + end_pos);
                        dir.destination = std::filesystem::path(dst_dir);
                    }
                    else{
                        logger log(App_MESSAGE("Syntax error missing ;"),Error::DEBUG);
                        log.to_console();
                        log.to_log_file();

                        // dir.destination is left blank and will be removed when the directories are checked
                    }
                    break;
                }
                case cs::close_brace:{
                    end = true;
                    m_Data->push_back(dir);
                    break;
                }
                default:{
                    break;
                }
            }
        }
    }
}

application::cs application::FileParse::ParseMonitorArgs(std::istringstream &lineStream)
{
    cs commands = cs::none;
    std::string token;
    while(lineStream >> token){
        auto found_token = global_tokenizer.Find(token);
        if(found_token.has_value()){
            switch(found_token.value()){
                case cs::sync:{
                    if((commands & cs::sync_add) == cs::none){
                        commands |= cs::sync;
                    }
                    break;
                }
                case cs::sync_add:{
                    if((commands & cs::sync)==cs::none){
                        commands |= cs::sync_add;
                    }
                    break;
                }
                case cs::recursive:{
                    if((commands & cs::single) == cs::none){
                        commands |= cs::recursive;
                    }
                    break;
                }
                case cs::single:{
                    if((commands & cs::recursive) == cs::none){
                        commands |= cs::single;
                    }
                    break;
                }
                case cs::overwrite:{
                    if((commands & cs::update) == cs::none){
                        commands |= cs::overwrite;
                    }
                    break;
                }
                case cs::update:{
                    if((commands & cs::overwrite) == cs::none){
                        commands |= cs::update;
                    }
                    break;
                }
                default:{
                    break;
                }
            }
        }
    }
    return commands;
}

application::cs application::FileParse::ParseBenchArgs(std::istringstream &lineStream)
{
    cs commands = cs::none;
    std::string token;
    while(lineStream >> token){
        auto found_token = global_tokenizer.Find(token);
        if(found_token.has_value()){
            switch(found_token.value()){
                case cs::create:{
                    commands |= cs::create;
                }
                case cs::four_k:{
                    commands |= cs::four_k;
                }
                case cs::fast:{
                    commands |= cs::fast;
                }
                default:{
                    break;
                }
            }
        }
    }
    return commands;
}
