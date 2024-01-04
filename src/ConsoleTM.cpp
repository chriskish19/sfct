#include "ConsoleTM.hpp"
//////////////////////////////////////////////////////////
/* narrow string version of ConsoleTM class definitions */
//////////////////////////////////////////////////////////

application::ConsoleTM::ConsoleTM(std::string message, int MaxLength)
:m_MaxLength(MaxLength){
    // set the maximum length of m_Message
    m_Message->reserve(m_MaxLength);

    // set m_Message to the constructor message
    *m_Message = message;

}

void application::ConsoleTM::to_console(){
    std::lock_guard<std::mutex> local_lock(m_Message_mtx);

    std::cout << "\033[K";

    // animate the output
    std::cout << "\r" << std::format("{} {}", m_AnimationChars[m_AnimationIndex++], *m_Message);

    // cycle the index from 0 to 4
    m_AnimationIndex %= 4;
}

void application::ConsoleTM::SetMessage(const std::string& m){
    std::lock_guard<std::mutex> local_lock(m_Message_mtx);

    m_Message->resize(m.size());

    // set m_Message to m
    *m_Message = m;
}

application::ConsoleTM::ConsoleTM(){
    // will be set to allow 1024 characters message length
    m_Message->reserve(m_MaxLength);
}

void application::ConsoleTM::RunMessages(){
    while(*m_Running){
        to_console();
        
        // pause execution for 150ms so the output animation is fluid
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}


/////////////////////////////////////////////////
/* wide string version of ConsoleTM definitions*/
/////////////////////////////////////////////////

void application::wConsoleTM::to_console(){
    std::lock_guard<std::mutex> local_lock(m_Message_mtx);

    if(!m_MessageQueue.empty()){
        // animate the output
        std::wcout << m_MessageQueue.front() << std::endl;

        m_MessageQueue.pop();
    }
    else{
        std::wcout << L"\r" <<  m_AnimationChars[m_AnimationIndex++];

        // cycle the index from 0 to 4
        m_AnimationIndex %= 4;
    }

    
}

void application::wConsoleTM::SetMessage(const std::wstring& m){
    std::lock_guard<std::mutex> local_lock(m_Message_mtx);

    m_MessageQueue.emplace(m);
}

void application::wConsoleTM::RunMessages(){
    while(*m_Running){
        to_console();

        // pause execution for 150ms so the output animation is fluid
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}