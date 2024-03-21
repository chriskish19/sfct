#include "w32directorycopy.hpp"

application::directory_copy::directory_copy(std::shared_ptr<std::vector<copyto>> dirs) noexcept
:m_dirs(dirs)
{
    m_init_success = initialize_dc();
}

void application::directory_copy::fast_copy() noexcept
{
    // if there was an error in the initialize_dc() function
    // we return since it would crash the program if we continued.
    if(!m_init_success){
        return;
    }
    
    // process each directory in m_dirs
    for(const auto& dir:*m_dirs){
        // get the size and number of files in the directory
        auto di = sfct_api::get_directory_info(dir);
        
        if(di.has_value()){
            // display the information to the console
            STDOUT << App_MESSAGE("Copying directory: ") << dir.source << App_MESSAGE(" to: ") << dir.destination << "\n";
            STDOUT << App_MESSAGE("Total size in bytes: ") << di.value().TotalSize << "\n";
            STDOUT << App_MESSAGE("Total number of files: ") << di.value().FileCount << "\n";
        }

        // create a benchmark to get the transfer speed
        benchmark test;

        // start the timer
        test.start_clock();

        // attempt to copy the directory
        sfct_api::copy_entry(dir.source,dir.destination,dir.co);
        
        // end the timer
        test.end_clock();

        double_t rate{};
        if(di.has_value()){
            // calculate the transfer speed
            rate = test.speed(di.value().TotalSize);
        }

        // display the transfer speed to the console
        STDOUT << App_MESSAGE("Transfer speed in MB/s: ") << rate << "\n";
    }
}

void application::directory_copy::copy() noexcept
{
    // if there was an error in the initialize_dc() function
    // we return since it would crash the program if we continued.
    if(!m_init_success){
        return;
    }

    // process each directory in m_dirs
    for(const auto& dir:*m_dirs){
        // get the size and number of files in the directory
        auto di = sfct_api::get_directory_info(dir);
        if(di.has_value()){
            // display the information to the console
            STDOUT << App_MESSAGE("Copying directory: ") << dir.source << App_MESSAGE(" to: ") << dir.destination << "\n";
            STDOUT << App_MESSAGE("Total size in bytes: ") << di.value().TotalSize << "\n";
            STDOUT << App_MESSAGE("Total number of files: ") << di.value().FileCount << "\n";
        }

        // create a benchmark to get the transfer speed
        benchmark test;

        // start the timer
        test.start_clock();

        // we check for recursive flag because we will use different iterators for the directories
        // for recursive and non-recursive
        if(sfct_api::recursive_flag_check(dir.commands)){
            
            // if this throws we return and exit the copy() function (any exception)
            // dont worry the threads are jthreads which auto join when out of scope
            try{
                TM worker;      // worker threads object

                // recursively iterate through dir.source
                // create a file_queue_info object for each file entry in the directory
                // then create a thread to process it in mt_process_file_queue_info_entry() function
                for(const auto& entry:std::filesystem::recursive_directory_iterator(dir.source)){
                    const auto dst_path = sfct_api::create_file_relative_path(entry.path(),dir.destination,dir.source,true);
                    if(dst_path.has_value()){                                               // if dst_path was successful
                        file_queue_info _file_info;                                         // create a file_queue_info object for processing
                        _file_info.co = dir.co;                                             // set the copy options
                        _file_info.dst = dst_path.value();                                  // set the dst path
                        _file_info.fqs = file_queue_status::file_added;                     // file_queue_status set as file_added
                        const auto gfs_src = sfct_api::get_file_status(entry.path());       // get the file_status of src path
                        _file_info.src = entry.path();                                      // set the src path

                        if(gfs_src.has_value()){                                            // if get_file_status was successful
                            _file_info.fs_src = gfs_src.value();                            // set the file_status
                        }

                        // give the work to a thread to process the entry
                        worker.do_work(&sfct_api::mt_process_file_queue_info_entry,_file_info);
                    }
                    else{
                        // inform the user that the entry will be skipped
                        logger log(App_MESSAGE("Skipping entry, failed to obtain relative path"),Error::WARNING,entry.path());
                        log.to_console();
                        log.to_log_file();
                    }

                    worker.join_one();              // join the oldest thread
                } 

                worker.join_all();                  // join all threads since we have completed
            }
            catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Filesystem error: " << e.what() << "\n";
                return;
            }
            catch(const std::runtime_error& e){
                std::cerr << "Runtime error: " << e.what() << "\n";
                return;
            }
            catch(const std::bad_alloc& e){
                std::cerr << "Allocation error: " << e.what() << "\n";
                return;
            }
            catch (const std::exception& e) {
                std::cerr << "Standard exception: " << e.what() << "\n";
                return;
            } 
            catch (...) {
                std::cerr << "Unknown exception caught \n";
                return;
            }
        }
        else{
            // if this throws we return and exit the copy() function (any exception)
            // dont worry the threads are jthreads which auto join when out of scope
            try{
                TM worker;          // worker threads object
                
                // iterate through dir.source
                // create a file_queue_info object for each file entry in the directory
                // then create a thread to process it in mt_process_file_queue_info_entry() function
                for(const auto& entry:std::filesystem::directory_iterator(dir.source)){
                    file_queue_info _file_info;                                     // create a file_queue_info object
                    _file_info.co = dir.co;                                         // set copy options
                    _file_info.dst = dir.destination;                               // set dst
                    _file_info.fqs = file_queue_status::file_added;                 // set file_queue_status as file_added
                    auto gfs_src = sfct_api::get_file_status(entry.path());         // get file_status
                    _file_info.src = entry.path();                                  // set src

                    if(gfs_src.has_value()){                                        // if get_file_status was successful
                        _file_info.fs_src = gfs_src.value();                        // set the file_status
                    }

                    // give the work to a thread to process the entry
                    worker.do_work(&sfct_api::mt_process_file_queue_info_entry,_file_info);
                    worker.join_one();      // join the oldest thread first
                }
                worker.join_all();          // join all since we have completed processing
            }
            catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Filesystem error: " << e.what() << "\n";
                return;
            }
            catch(const std::runtime_error& e){
                std::cerr << "Runtime error: " << e.what() << "\n";
                return;
            }
            catch(const std::bad_alloc& e){
                std::cerr << "Allocation error: " << e.what() << "\n";
                return;
            }
            catch (const std::exception& e) {
                std::cerr << "Standard exception: " << e.what() << "\n";
                return;
            } 
            catch (...) {
                std::cerr << "Unknown exception caught \n";
                return;
            }
        
        }
        // end the timer
        test.end_clock();

        double_t rate{};
        if(di.has_value()){                             // get_directory_info was successful
            rate = test.speed(di.value().TotalSize);    // calculate the rate
        }

        // display the transfer rate to the console
        STDOUT << "\n";
        STDOUT << App_MESSAGE("Transfer speed in MB/s: ") << rate << "\n";
    }
    
}

bool application::directory_copy::initialize_dc() noexcept
{
    if(!m_dirs){
        // log the error to the console
        logger log(App_MESSAGE("nullptr"),Error::FATAL);
        log.to_console();
        log.to_log_file();
        return false;
    }
    return true;
}
