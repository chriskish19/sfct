#include "directory_copy.hpp"

application::directory_copy::directory_copy(std::shared_ptr<std::vector<copyto>> dirs) noexcept
:m_dirs(dirs)
{
    try{
        if(!dirs){
            logger log(App_MESSAGE("nullptr"),Error::FATAL);
            log.to_console();
            log.to_log_file();
            // make a blank object so directory copy can exit safely
            m_dirs = std::make_shared<std::vector<copyto>>();
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem related errors
        std::cerr << "Filesystem error: " << e.what() << "\n";
    }
    catch(const std::runtime_error& e){
        // the error message
        std::cerr << "Runtime error: " << e.what() << "\n";
    }
    catch(const std::bad_alloc& e){
        // the error message
        std::cerr << "Allocation error: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        // Catch other standard exceptions
        std::cerr << "Standard exception: " << e.what() << "\n";
    } catch (...) {
        // Catch any other exceptions
        std::cerr << "Unknown exception caught \n";
    }
}

void application::directory_copy::fast_copy() noexcept
{
    for(const auto& dir:*m_dirs){
        auto di = sfct_api::get_directory_info(dir);
        if(di.has_value()){
            STDOUT << App_MESSAGE("Copying directory: ") << dir.source << App_MESSAGE(" to: ") << dir.destination << "\n";
            STDOUT << App_MESSAGE("Total size in bytes: ") << di.value().TotalSize << "\n";
            STDOUT << App_MESSAGE("Total number of files: ") << di.value().FileCount << "\n";
        }

        benchmark test;
        test.start_clock();
        sfct_api::copy_entry(dir.source,dir.destination,dir.co);
        test.end_clock();

        double_t rate{};
        if(di.has_value()){
            rate = test.speed(di.value().TotalSize);
        }

        STDOUT << App_MESSAGE("Transfer speed in MB/s: ") << rate << "\n";
    }
}

void application::directory_copy::copy() noexcept
{

    for(const auto& dir:*m_dirs){
        
        auto di = sfct_api::get_directory_info(dir);
        if(di.has_value()){
            STDOUT << App_MESSAGE("Copying directory: ") << dir.source << App_MESSAGE(" to: ") << dir.destination << "\n";
            STDOUT << App_MESSAGE("Total size in bytes: ") << di.value().TotalSize << "\n";
            STDOUT << App_MESSAGE("Total number of files: ") << di.value().FileCount << "\n";
        }


        benchmark test;
        test.start_clock();
        if(sfct_api::recursive_flag_check(dir.commands)){

            try{
                TM worker;
                for(const auto& entry:std::filesystem::recursive_directory_iterator(dir.source)){
                    auto dst_path = sfct_api::create_file_relative_path(entry.path(),dir.destination,dir.source,true);
                    if(dst_path.has_value()){
                        file_queue_info _file_info;
                        _file_info.co = dir.co;
                        _file_info.dst = dst_path.value();
                        _file_info.fqs = file_queue_status::file_added;
                        auto gfs_src = sfct_api::get_file_status(entry.path());
                        _file_info.src = entry.path();

                        if(gfs_src.has_value()){
                            _file_info.fs_src = gfs_src.value();
                        }

                        worker.do_work(&sfct_api::mt_process_file_queue_info_entry,_file_info);
                    }
                    else{
                        logger log(App_MESSAGE("Skipping entry, failed to obtain relative path"),Error::WARNING,entry.path());
                        log.to_console();
                        log.to_log_file();
                    }

                    worker.join_one();
                } 

                worker.join_all();
            }
            catch (const std::filesystem::filesystem_error& e) {
                // Handle filesystem related errors
                std::cerr << "Filesystem error: " << e.what() << "\n";

                return;
            }
            catch(const std::runtime_error& e){
                // the error message
                std::cerr << "Runtime error: " << e.what() << "\n";

                return;
            }
            catch(const std::bad_alloc& e){
                // the error message
                std::cerr << "Allocation error: " << e.what() << "\n";

                return;
            }
            catch (const std::exception& e) {
                // Catch other standard exceptions
                std::cerr << "Standard exception: " << e.what() << "\n";

                return;
            } catch (...) {
                // Catch any other exceptions
                std::cerr << "Unknown exception caught \n";

                return;
            }


        }
        else{

            try{
                TM worker;
                for(const auto& entry:std::filesystem::directory_iterator(dir.source)){
                    file_queue_info _file_info;
                    _file_info.co = dir.co;
                    _file_info.dst = dir.destination;
                    _file_info.fqs = file_queue_status::file_added;
                    auto gfs_src = sfct_api::get_file_status(entry.path());
                    _file_info.src = entry.path();

                    if(gfs_src.has_value()){
                        _file_info.fs_src = gfs_src.value();
                    }

                    worker.do_work(&sfct_api::mt_process_file_queue_info_entry,_file_info);
                    worker.join_one();
                }
                worker.join_all();
            }
            catch (const std::filesystem::filesystem_error& e) {
                // Handle filesystem related errors
                std::cerr << "Filesystem error: " << e.what() << "\n";

                return;
            }
            catch(const std::runtime_error& e){
                // the error message
                std::cerr << "Runtime error: " << e.what() << "\n";

                return;
            }
            catch(const std::bad_alloc& e){
                // the error message
                std::cerr << "Allocation error: " << e.what() << "\n";

                return;
            }
            catch (const std::exception& e) {
                // Catch other standard exceptions
                std::cerr << "Standard exception: " << e.what() << "\n";

                return;
            } catch (...) {
                // Catch any other exceptions
                std::cerr << "Unknown exception caught \n";

                return;
            }
        
        }
        test.end_clock();

        double_t rate{};
        if(di.has_value()){
            rate = test.speed(di.value().TotalSize);
        }

        STDOUT << "\n";
        STDOUT << App_MESSAGE("Transfer speed in MB/s: ") << rate << "\n";
    }
    
}


