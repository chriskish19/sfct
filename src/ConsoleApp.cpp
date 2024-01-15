#include "ConsoleApp.hpp"

application::ConsoleApp::ConsoleApp(){
#if WINDOWS_BUILD
    Windows::SetWideConsoleMode();
#endif

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
    
    // make a monitor for directories
    m_Monitor = std::make_unique<DirectorySignal>(m_data);
}

void application::ConsoleApp::Go(){
    std::thread MessageStreamThread(&application::CONSOLETM::RunMessages,&m_MessageStream);
    
    




    

    *m_MessageStream.GetSPRunning() = false;

    if(MessageStreamThread.joinable()){
        MessageStreamThread.join();
    }
}



