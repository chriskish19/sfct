#include "ConsoleApp.hpp"

application::ConsoleApp::ConsoleApp(){
    // Open the file for reading
    // The FileParse class does not handle file not found it just returns false
    if(!m_List.OpenFile()){
        std::fstream FileOut(m_FileName,std::ios::out);
        throw std::runtime_error(m_FileName + " file not found, creating file in current working directory, program will exit now");
    }

    // parse the file now
    m_List.ExtractData();

    // check the data for valid entries
    // FileParse handles the case where no valid directories are found and
    // throws a std::runtime_error exception
    m_List.CheckData();

    // get the data shared_ptr for use in the console app class
    // it holds the directory paths
    m_data = m_List.GetSPdata();
    
    
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

void application::ConsoleApp::Go(){
    std::thread MessageStreamThread(&application::CONSOLETM::RunMessages,&m_MessageStream);
    
    if(!m_copy_dirs->empty()){
        m_MessageStream.SetMessage(App_MESSAGE("Preparing to copy files"));
        m_MessageStream.SetMessage(App_MESSAGE("Checking files..."));
        m_MessageStream.ReleaseBuffer();

        directory_copy dc(m_copy_dirs);
        dc.copy();
    }
    
    if(!m_fast_copy_dirs->empty()){
        m_MessageStream.SetMessage(App_MESSAGE("Preparing to fast copy files"));
        m_MessageStream.ReleaseBuffer();

        directory_copy dc(m_fast_copy_dirs);
        dc.fast_copy();
    }

    if(!m_bench_dirs.empty()){
        m_MessageStream.SetMessage(App_MESSAGE("Preparing to benchmark"));
        m_MessageStream.ReleaseBuffer();

        benchmark test;
        test.speed_test_directories(m_bench_dirs);
    }

    if(!m_monitor_dirs->empty()){
        m_MessageStream.SetMessage(App_MESSAGE("Preparing to monitor"));
        m_MessageStream.ReleaseBuffer();

        // make a monitor for directories
        m_Monitor = std::make_unique<DirectorySignal>(m_monitor_dirs);
        
        // monitor directories
        m_Monitor->monitor();
    }
   

    m_MessageStream.SetMessage(App_MESSAGE("Exiting"));

    // end the message stream
    m_MessageStream.end();

    if(MessageStreamThread.joinable()){
        MessageStreamThread.join();
    }
}



