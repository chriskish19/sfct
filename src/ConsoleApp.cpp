#include "ConsoleApp.hpp"

application::ConsoleApp::ConsoleApp(){
    // set copy options
    std::filesystem::copy_options co = std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing;

    // Note: class member functions need to be called to actually parse the data
    // into the vector thats passed as a shared_ptr to m_Copier
    // InitializationCheck() will contain the calls to parse the data since its bad practice to 
    // throw exceptions from constructors
    m_List = std::make_unique<FileParse>(m_FileName);

    
    m_Copier = std::make_unique<FileCopy>();

    // get the number of worker threads that are going to be used
    size_t max_workers = m_MessageWorkers->GetNumberOfWorkers();

    // now build that many ConsoleTM objects for each worker
    for(size_t i{};i<max_workers;i++){
        m_MessageStreams->push_back(ConsoleTM());
    }

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
}

void application::ConsoleApp::Go(){
    while(true){
        
    }
}

