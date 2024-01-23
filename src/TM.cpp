#include "TM.hpp"

void application::TM::join_all(){
    for(auto& t:m_Threads){
        if(t.joinable()){
            t.join();
        }
    }

    m_Threads.clear();
}

application::SystemPerformance application::TM::GetPCspec() noexcept{
    // cant use m_TotalThreads because there is no guarantee of initialization order
    // of static class members
    unsigned int total_threads = std::thread::hardware_concurrency();
    
    // set PC_SPEC for system
    if(total_threads<5){ // 1 to 4
        return SystemPerformance::SLOW;
    }
    else if(total_threads > 4 && total_threads < 9){ // 5 to 8
        return SystemPerformance::AVERAGE;
    } 
    else{
        return SystemPerformance::FAST;
    }
}

application::TM::TM(){
    SetWorkers();
}

void application::TM::SetWorkers(){
    switch(m_PC_SPEC){
        case SystemPerformance::SLOW:{
            m_Workers = 1;
            break;
        }
        case SystemPerformance::AVERAGE:{
            m_Workers = 4;
            break;
        }
        case SystemPerformance::FAST:{
            m_Workers = 8;
            break;
        }
        default:{
            m_Workers = 1;
            break;
        }
    }
}

bool application::TM::join_one(){
    // only allow m_Workers threads to run at a time
    if (m_Threads.size() >= m_Workers && m_Threads.front().joinable()) {
        m_Threads.front().join();  // Join the oldest thread
        m_Threads.erase(m_Threads.begin());  // Remove it from the vector
        return true;
    }
    return false;
}