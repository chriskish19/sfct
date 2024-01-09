#include "ConsoleApp.hpp"

application::ConsoleApp::ConsoleApp(){
#if WINDOWS_BUILD
    Windows::SetWideConsoleMode();
#endif
    
    // Note: class member functions need to be called to actually parse the data
    // into the vector thats passed as a shared_ptr to m_Copier
    m_List = std::make_unique<FileParse>(m_FileName);

    // get the number of worker threads that are going to be used
    size_t max_workers = m_CopyWorkers->GetNumberOfWorkers();

    // Open the file for reading
    // The FileParse class does not handle file not found it just returns false
    if(!m_List->OpenFile()){
        std::fstream FileOut(m_FileName,std::ios::out);
        throw std::runtime_error(m_FileName + " file not found, creating file in current working directory, program will exit now");
    }

    // parse the file now
    m_List->ExtractData(m_Keyword);

    // check the data for valid entries
    // FileParse handles the case where no valid directories are found and
    // throws a std::runtime_error exception
    m_List->CheckData(DataType::Directory);

    // get the data shared_ptr for use in the console app class
    // it holds the directory paths
    m_data = m_List->GetSPdata();
    
    // make a monitor for directories
    m_Monitor = std::make_unique<DirectorySignal>(m_data);

    // get a file queue shared_ptr from DirectorySignal class
    m_FileQueue = m_Monitor->GetFileQueueSP();
}

void application::ConsoleApp::Go(){
    FullCopy(m_data);

    // main thread monitors directories indefinitely
    m_Monitor->monitor();
}



