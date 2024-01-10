#include "ConsoleTM.hpp"
//////////////////////////////////////////////////////////
/* narrow string version of ConsoleTM class definitions */
//////////////////////////////////////////////////////////

void application::ConsoleTM::to_console(){
    std::lock_guard<std::mutex> local_lock(m_Message_mtx);

    if(!m_MessageQueue.empty()){
        // animate the output
        std::cout << m_MessageQueue.front() << "\n";

        m_MessageQueue.pop();
    }
    else{
        std::cout << "\r" <<  m_AnimationChars[m_AnimationIndex++];

        // cycle the index from 0 to 4
        m_AnimationIndex %= 4;
    }
}

void application::ConsoleTM::SetMessage(const std::string& m){
    std::lock_guard<std::mutex> local_lock(m_Message_mtx);
    m_MessageQueue.emplace(m);
}

void application::ConsoleTM::RunMessages(){
    while(*m_Running){
        to_console();

        // pause execution for 150ms so the output animation is fluid
        // and messages can be added to the queue
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}


/////////////////////////////////////////////////
/* wide string version of ConsoleTM definitions*/
/////////////////////////////////////////////////

void application::wConsoleTM::to_console(){
    
    if(m_release){
        std::lock_guard<std::mutex> local_lock(m_Message_mtx);
        while(!m_MessageQueue.empty()){
            std::wcout << m_MessageQueue.front() << "\n";

            m_MessageQueue.pop();
        }
        m_release = false;
    }
    else{
        // animate the output
        std::wcout << L"\r" <<  m_AnimationChars[m_AnimationIndex++];

        // cycle the index from 0 to 4
        m_AnimationIndex %= 4;

        // pause execution for 150ms so the output animation is fluid
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

void application::wConsoleTM::SetMessage(const std::wstring& m){
    std::lock_guard<std::mutex> local_lock(m_Message_mtx);
    m_MessageQueue.emplace(m);
}

void application::wConsoleTM::RunMessages(){
    while(*m_Running){
        to_console();
    }
}

void application::wConsoleTM::ReleaseBuffer(){
    m_release = true;
}