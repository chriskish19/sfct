#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <filesystem>
#include <thread>
#include <chrono>


struct copyto{
    std::wstring source;
    std::wstring destination;
    std::filesystem::file_time_type time_modified;
};

void copy_process(std::vector<copyto> &dirs);

int main(){
    // open the sfct folder list file for parsing
    std::wifstream f_list{L"sfct_list.txt",std::ios::in};
    if(f_list.fail()){
        std::puts("Failed to open sfct list of directories for cloning \n");
        std::puts("Creating file sfct_list.txt in the exe directory. See docs for example usage \n");
        std::ofstream CreateList{"sfct_list.txt",std::ios::out};
        return -1;
    }

    // make a copyto vector that holds both sources and destinations
    std::vector<copyto> directories;

    std::wstring line;
    while(std::getline(f_list,line)){
        std::wistringstream lineStream(line);
        std::wstring prefix;
        lineStream >> prefix;

        if(prefix == L"copy"){
            copyto directory;
            lineStream >> directory.source >> directory.destination;
            directories.push_back(directory);
        }
    }


    // check that the directories are valid
    // if the source entry is not valid mark the entry to be removed
    // if the destination does not exist it will be created
    for(size_t i{};i<directories.size();){
        if(!std::filesystem::exists(directories.at(i).source)){
            std::puts("The copy directory is invalid, its entry will not be used \n");
            std::wcout << directories.at(i).source << "\n";

            // erase the entry
            // Do not increment i, as the next element has shifted to the current position
            directories.erase(directories.begin() + i);
        }
        else{
            // Only increment i if an element was not erased
            i++;
        }
    }
    
    // check that the destination exists and if it does not, attempt to create it
    for(size_t i{};i<directories.size();i++){
        if(!std::filesystem::exists(directories.at(i).destination)){
            if(!std::filesystem::create_directories(directories.at(i).destination)){
                std::puts("Error: failed to create directories, to is an invalid path \n");
                std::wcout << directories.at(i).destination << "\n";

                // mark the directory as invalid
                directories.at(i).destination = L"INVALID";
                directories.at(i).source = L"INVALID";
            }
        }
    }

    // erase the invalid directories
    for(size_t i{};i<directories.size();){
        if(directories.at(i).destination == L"INVALID"){
            directories.erase(directories.begin()+i);
        }
        else{
            i++;
        }
    }

    // check that directories has entries
    if(directories.empty()){
        std::puts("Error: No valid directories in sfct_list.txt \n");
        return -1;
    }

    
    // intial copying process
    copy_process(directories);


    // use a listener and update the directories when a new file is added or existing is changed
    // periodically check the directory write times, if they have changed then initiate the copy process
    bool begin_copy{false};
    while(true){
        const auto file_system_time = std::filesystem::file_time_type::clock::now();

        // wait five seconds
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    
        for(size_t i{};i<directories.size();i++){
            // get the latest write times
            directories.at(i).time_modified = std::filesystem::last_write_time(directories.at(i).source);
            const auto modified_file_time = directories.at(i).time_modified; 
            
            if(file_system_time < modified_file_time){
                begin_copy = true;
            }
        }

        if(begin_copy){
            copy_process(directories);
        }
        
        // reset
        begin_copy = false;
        
    }





    return 0;
}

void copy_process(std::vector<copyto> &dirs){
    const auto copyOptions = std::filesystem::copy_options::update_existing | std::filesystem::copy_options::recursive;
    
    // TODO: multithread this
    // start the copy operations
    for(size_t i{};i<dirs.size();i++){
        std::filesystem::copy(dirs.at(i).source,dirs.at(i).destination,copyOptions);

        // set the intial write times
        dirs.at(i).time_modified = std::filesystem::last_write_time(dirs.at(i).source);
    }
}