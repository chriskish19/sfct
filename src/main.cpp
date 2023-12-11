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
#include <stdexcept>
#include <functional>

struct copyto{
    std::wstring source;
    std::wstring destination;
    std::filesystem::path fs_source;
    std::filesystem::path fs_destination;
};

// main copy process
void copy_process(std::vector<copyto> &dirs, std::shared_ptr<std::string> cm, std::shared_ptr<std::string> fm, std::filesystem::copy_options co);

// console messages and animation
void output_animation(std::shared_ptr<std::atomic<bool>> run, std::shared_ptr<std::string> s);

// copy an individual file useful with threads
void copyFile(const std::filesystem::path& source, const std::filesystem::path& destination, std::filesystem::copy_options options);


// to prevent concurrent access to the shared_ptr<std::string> console_message
std::mutex console_message_mutex;

// to prevent concurrent access to the shread_ptr<std::string> CurrentFilepathCopied
std::mutex CurrentFilepathCopied_mtx;

// global pc specs for running copy operations
// if the system is slow then copy operations are slowed down
// if the system is considered fast then copy operations are not slowed down
SystemPerformance PC_SPEC;

enum class SystemPerformance{
    SLOW,
    AVERAGE,
    FAST
};

int main(){

    // check hardware concurrency, avaliable threads
    unsigned int total_threads = std::thread::hardware_concurrency();

    // set PC_SPEC for system
    if(total_threads<5){ // 1 to 4
        PC_SPEC = SystemPerformance::SLOW;
    }
    else if(total_threads > 4 && total_threads < 9){ // 5 to 8
        PC_SPEC = SystemPerformance::AVERAGE;
    } 
    else{
        PC_SPEC = SystemPerformance::FAST;
    }

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


    // initialize fs_source and fs_destination in the directories vector
    // using the std::wstring's source and destination
    // std::filesystem works best with its native path type
    for(size_t i{};i<directories.size();i++){
        std::filesystem::path src_init(directories.at(i).source);
        std::filesystem::path dest_init(directories.at(i).destination);
        directories.at(i).fs_source = src_init;
        directories.at(i).fs_destination = dest_init;
    }


    // use a listener and update the directories when a new file is added or existing is changed
    // periodically check the directory write times, if they have changed then initiate the copy process
    bool begin_copy{false},writetimeExit{false};

    std::shared_ptr<std::atomic<bool>> running{std::make_shared<std::atomic<bool>>(true)};
    std::shared_ptr<std::string> console_message{std::make_shared<std::string>()};
    
    // this will be used to display the full file name and path that is 
    // currently being copied
    std::shared_ptr<std::string> CurrentFilepathCopied{std::make_shared<std::string>()};

    // set capacity of CurrentFilepathCopied
    // on UNIX systems 4096 is the limit
    // ON windows typically 260 but can be extended to >32000
    CurrentFilepathCopied->reserve(4096);

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
            for(const auto& entry : std::filesystem::recursive_directory_iterator(directories.at(i).fs_source)){
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

// std::vector<copyto> &dirs : vector containing directories to copy
// cm: console message to display
// fm: file path message to display current file being copied
// co: copy options ,example would be std::filesystem::copy_options::update_existing enum flag
void copy_process(std::vector<copyto> &dirs, std::shared_ptr<std::string> cm, std::shared_ptr<std::string> fm, std::filesystem::copy_options co){
    // start the copy operations
    for(size_t i{};i<dirs.size();i++){
        // update console message
        // thread saftey
        console_message_mutex.lock();
        *cm = "Copying Files";
        console_message_mutex.unlock();

        try{
            // check co flag for recursive copy 
            if((co & std::filesystem::copy_options::recursive) != std::filesystem::copy_options::none){

            }
            else{
                switch(PC_SPEC){
                    case SystemPerformance::SLOW:{
                        // single threaded regular copy
                        // get a directory iterator for displaying current file being copied
                        for(const auto& file_entry: std::filesystem::directory_iterator(dirs.at(i).fs_source)){
                            // to display the file path to the console
                            CurrentFilepathCopied_mtx.lock();
                            *fm = file_entry.path().string();
                            CurrentFilepathCopied_mtx.unlock();
                            
                            // copy
                            std::filesystem::copy_file(file_entry.path(),dirs.at(i).fs_destination,co);
                        }
                        break;
                    }
                    case SystemPerformance::AVERAGE:{ // safe to use 4 threads to copy the files
                        // vector to hold the created threads
                        std::vector<std::thread> threads;
                        
                        // multithreaded copy
                        // get a directory iterator for displaying current file being copied
                        for(const auto& file_entry: std::filesystem::directory_iterator(dirs.at(i).fs_source)){
                            // to display the file path to the console
                            // it wont be able to display the exact file being copied but
                            // instead when it was started
                            // in the future I will change this so that the console will display 
                            // all working threads and files currently being copied
                            // TODO: display all files currently being copied by x threads
                            CurrentFilepathCopied_mtx.lock();
                            *fm = file_entry.path().string();
                            CurrentFilepathCopied_mtx.unlock();
                            
                            // only allow 4 threads to run at a time
                            if (threads.size() >= 4) {
                                threads.front().join();  // Join the oldest thread
                                threads.erase(threads.begin());  // Remove it from the vector
                            }
                            
                            // create a thread to run the copyFile function
                            threads.emplace_back(copyFile,file_entry.path(),dirs.at(i).fs_destination,co);
                        }

                        // Join any remaining threads
                        for (auto& t : threads) {
                            if (t.joinable()) {
                                t.join();
                            }
                        }

                        break;
                    }
                    case SystemPerformance::FAST:{ // safe to use 8 threads to copy the files
                        // vector to hold the created threads
                        std::vector<std::thread> threads;
                        
                        // multithreaded copy
                        // get a directory iterator for displaying current file being copied
                        for(const auto& file_entry: std::filesystem::directory_iterator(dirs.at(i).fs_source)){
                            // to display the file path to the console
                            CurrentFilepathCopied_mtx.lock();
                            *fm = file_entry.path().string();
                            CurrentFilepathCopied_mtx.unlock();
                            
                            // only allow 8 threads to run at a time
                            // Check if we need to join any threads
                            if (threads.size() >= 8) {
                                threads.front().join();  // Join the oldest thread
                                threads.erase(threads.begin());  // Remove it from the vector
                            }
                            
                            // create a thread to run the copyFile function
                            threads.emplace_back(copyFile,file_entry.path(),dirs.at(i).fs_destination,co);
                        }

                        // Join any remaining threads
                        for (auto& t : threads) {
                            if (t.joinable()) {
                                t.join();
                            }
                        }

                        break;
                    }
                    default:{
                        // if PC_SPEC is not initialized return to the caller
                        return;
                    }

                }
            }
            
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << '\n';
            std::cerr << "Error code: " << e.code() << '\n';
        } catch (const std::exception& e) {
            // This catches other standard exceptions
            std::cerr << "Standard exception: " << e.what() << '\n';
        } catch (...) {
            // This catches any other exceptions
            std::cerr << "An unknown error occurred.\n";
        }
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
        std::cout << "\r" << std::format("{} {}", animationChars[animationIndex++], *s) << std::flush;
        
        // unlock mutex
        console_message_mutex.unlock();

        // TODO: output the current file being copied
        // 

        

        // cycle the index from 0 to 4
        animationIndex %= 4;

        // pause execution for 50ms so the output animation is fluid
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void copyFile(const std::filesystem::path& source, const std::filesystem::path& destination, std::filesystem::copy_options options) {
    try {
        std::filesystem::copy_file(source, destination, options);
        // If you want, you can add code here to indicate success.
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to copy file: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.code() << '\n';
        // Handle the error, for example, logging it or setting an error flag.
    }
    // Additional error handling if necessary.
}