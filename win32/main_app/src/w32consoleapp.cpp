#include "w32consoleapp.hpp"

application::ConsoleApp::ConsoleApp() noexcept{
    m_init_success = initialize_app();
}

void application::ConsoleApp::Go() noexcept{
    // if initialize_app() fails return and exit the program
    if(!m_init_success){
        return;
    }

    // if an exception is thrown(any exception) return and exit the program
    try{
        if(!m_copy_dirs->empty()){
            STDOUT << App_MESSAGE("Preparing to copy files \n");

            directory_copy dc(m_copy_dirs);
            dc.copy();
        }
        
        if(!m_fast_copy_dirs->empty()){
            STDOUT << App_MESSAGE("Preparing to fast copy files \n");

            directory_copy dc(m_fast_copy_dirs);
            dc.fast_copy();
        }

        if(!m_bench_dirs.empty()){
            STDOUT << App_MESSAGE("Preparing to benchmark \n");

            benchmark test;
            test.speed_test_directories(m_bench_dirs);
        }

        if(!m_monitor_dirs->empty()){
            STDOUT << App_MESSAGE("Preparing to monitor \n");

            // make a monitor for directories
            m_Monitor = std::make_unique<DirectorySignal>(m_monitor_dirs);
            
            // monitor directories
            m_Monitor->monitor();
        }
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

bool application::ConsoleApp::initialize_app() noexcept
{
    // Open the file for reading
    // The FileParse class does not handle file not found it just returns false
    if(!m_List.OpenFile()){
        // create the file in current directory
        std::fstream FileOut(m_FileName,std::ios::out);

        // log the error
        logger log(App_MESSAGE("file not found, creating file in current working directory, program will exit now"),Error::FATAL);
        log.to_console();
        log.to_log_file();
        return false;
    }

    // parse the file now
    m_List.ExtractData();

    // check the data for valid entries
    // if there is no valid directories then m_data vector will be empty.
    m_List.CheckData();

    // get the data shared_ptr for use in the console app class
    // it holds the directory paths
    m_data = m_List.GetSPdata();
    
    
    // if this throws(any exception) we return false and 
    // in the Go() function we return early and exit the program
    try{
        for(const auto& dir:*m_data){
            if((dir.commands & cs::monitor) != cs::none){
                m_monitor_dirs->push_back(dir);
            }

            if((dir.commands & cs::copy) != cs::none){
                m_copy_dirs->push_back(dir);
            }

            if((dir.commands & cs::fast_copy) != cs::none){
                m_fast_copy_dirs->push_back(dir);
            }

            if((dir.commands & cs::benchmark) != cs::none){
                m_bench_dirs.push_back(dir);
            }
        }
    }
    catch(const std::runtime_error& e){
        std::cerr << "Runtime error: " << e.what() << "\n";

        return false;
    }
    catch(const std::bad_alloc& e){
        std::cerr << "Allocation error: " << e.what() << "\n";

        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << "\n";

        return false;
    } 
    catch (...) {
        std::cerr << "Unknown exception caught \n";

        return false;
    }

    return true;
}
