#include "ConsoleTM.hpp"

application::ConsoleTM::ConsoleTM(std::string message, int MaxLength)
:m_MaxLength(MaxLength){
    // set the maximum length of m_Message
    m_Message->reserve(m_MaxLength);

    // set m_Message to the constructor message
    *m_Message = message;

}

void application::ConsoleTM::to_console()
{
     // clear the line
    std::cout << "\r" << m_ClearLine << "\r";
    
    m_Message_mtx.lock();

    // animate the output
    std::cout << "\r" << std::format("{} {}", m_AnimationChars[m_AnimationIndex++], *m_Message) << std::flush;

    m_Message_mtx.unlock();

    // cycle the index from 0 to 4
    m_AnimationIndex %= 4;
}

void application::ConsoleTM::SetMessage(std::string m){
    // if m is larger than m_MaxLength, return to the caller
    // do not attempt to set the m_Message shared_ptr value it points to
    if(m.size()>m_MaxLength){
        return;
    }

    // thread saftey
    m_Message_mtx.lock();

    // set m_Message to m
    *m_Message = m;

    m_Message_mtx.unlock();
}

application::ConsoleTM::ConsoleTM(){
    // will be set to allow 1024 characters message length
    m_Message->reserve(m_MaxLength);
}

void application::ConsoleTM::RunMessages(){
    while(*m_Running){
        to_console();
        
        // pause execution for 50ms so the output animation is fluid
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}