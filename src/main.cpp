#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <format>
#include <atomic>
#include <memory>
#include <mutex>

struct copyto{
    std::wstring source;
    std::wstring destination;
    std::filesystem::file_time_type time_modified;
};

void copy_process(std::vector<copyto> &dirs,std::shared_ptr<std::string> cm);
void output_animation(std::shared_ptr<std::atomic<bool>> run, std::shared_ptr<std::string> s);

// to prevent concurrent access to the shared_ptr<std::string> console_message
std::mutex console_message_mutex;

int main(){
    // open the sfct folder list file for parsing
    std::wifstream f_list{L"sfct_list.txt",std::ios::in};
    if(f_list.fail()){
        std::puts("Failed to open sfct list of directories for cloning");
        std::puts("Creating file sfct_list.txt in the exe directory. See docs for example usage");
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
        std::puts("Error: No valid directories in sfct_list.txt");
        return -1;
    }


    // use a listener and update the directories when a new file is added or existing is changed
    // periodically check the directory write times, if they have changed then initiate the copy process
    bool begin_copy{false},writetimeExit{false};

    std::shared_ptr<std::atomic<bool>> running{std::make_shared<std::atomic<bool>>(true)};
    std::shared_ptr<std::string> console_message{std::make_shared<std::string>()};
    
    // set capacity to 50 chars
    console_message->reserve(50);

    // output to the console on separate thread
    // the thread runs the lifetime of the program
    // console_message is changed throughout the the while loop depending on the 
    // current action
    std::thread t1(output_animation,running,console_message);
    
    // prevent concurrent access to console_message
    console_message_mutex.lock();
    
    *console_message = "Initial Copying Process";
    
    // safe to unlock
    console_message_mutex.unlock();
    
    // initial copy process
    copy_process(directories,console_message);

    while(true){
        const auto file_system_time = std::filesystem::file_time_type::clock::now();

        // prevent concurrent access to console_message
        console_message_mutex.lock();

        // set console message
        *console_message = "Waiting";

        // safe to unlock
        console_message_mutex.unlock();

        // wait 15 seconds
        std::this_thread::sleep_for(std::chrono::milliseconds(15000));
        
            
        for(size_t i{};i<directories.size() && !writetimeExit ;i++){
            // prevent concurrent access to console_message
            console_message_mutex.lock();
            
            // set console message to the current action
            *console_message = "Checking Directories";

            // safe to unlock
            console_message_mutex.unlock();


            // check the latest write times
            for(const auto& entry : std::filesystem::recursive_directory_iterator(directories.at(i).source)){
                // prevent concurrent access to console_message
                console_message_mutex.lock();

                *console_message = "Recursivley Checking Directories";
                
                // safe to unlock
                console_message_mutex.unlock();


                if(entry.last_write_time() > file_system_time ){
                    begin_copy = true;
                    writetimeExit = true;
                    break;
                }
            }
        }

        if(begin_copy){
            copy_process(directories,console_message);
        }
        
        // reset
        begin_copy = false;
        writetimeExit = false;
        
    }

    *running = false;
    if(t1.joinable()){
        t1.join();
    }



    return 0;
}

void copy_process(std::vector<copyto> &dirs, std::shared_ptr<std::string> cm){
    const auto copyOptions = std::filesystem::copy_options::update_existing | std::filesystem::copy_options::recursive;

    // start the copy operations
    for(size_t i{};i<dirs.size();i++){
        // update console message
        // thread saftey
        console_message_mutex.lock();
        *cm = "Copying Files";
        console_message_mutex.unlock();

        // copy the files
        std::filesystem::copy(dirs.at(i).source,dirs.at(i).destination,copyOptions);

        // set the intial write times
        dirs.at(i).time_modified = std::filesystem::last_write_time(dirs.at(i).source);
    }
}

void output_animation(std::shared_ptr<std::atomic<bool>> run, std::shared_ptr<std::string> s){
    char animationChars[4] = {'/', '-', '\\', '|'};
    int animationIndex = 0;
    std::string clearLine(50, ' '); // Line of 50 spaces

    while(*run){
        // prevent concurrent access
        console_message_mutex.lock();
        
        // clear the line
        std::cout << "\r" << clearLine << "\r";

        // animate the output
        std::cout << "\r" << std::format("{} {}", *s, animationChars[animationIndex++]) << std::flush;
        
        // unlock mutex
        console_message_mutex.unlock();

        // cycle the index from 0 to 4
        animationIndex %= 4;

        // pause execution for 50ms so the output animation is fluid
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}