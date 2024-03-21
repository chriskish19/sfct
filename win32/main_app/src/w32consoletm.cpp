#include "w32consoletm.hpp"


//////////////////////////////////////////////////////////
/* narrow string version of ConsoleTM class definitions */
//////////////////////////////////////////////////////////

void application::ConsoleTM::to_console() noexcept{
    // << operator can throw
    // sleep_for() can throw
    // if they do we exit the to_console() function (any exception) and exit the while loop
    // in RunMessages() by setting m_Running to false
    try{
        // if m_release is true then m_MessageQueue gets displayed to the console
        // else the animation is displayed
        if(m_release){
            // prevent concurrent access to m_MessageQueue
            std::lock_guard<std::mutex> local_lock(m_Message_mtx);
            
            // process the queue until its empty
            while(!m_MessageQueue.empty()){
                // display the top message to the console
                std::cout << m_MessageQueue.front() << "\n";

                // remove the top message
                m_MessageQueue.pop();
            }

            // m_MessageQueue is now empty
            // so reset m_release to false
            m_release = false;

            // notify the main thread if its waiting that processing has completed
            // and its safe to exit
            m_main_thread_cv.notify_one();
        }
        else{
            // ("/r")carriage return the animation so it displays correctly
            std::cout << "\r" <<  m_AnimationChars[m_AnimationIndex++];

            // cycle the index from 0 to 4
            m_AnimationIndex %= 4;

            // pause execution for 150ms so the output animation is fluid
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }        
    }
    catch(const std::runtime_error& e){
        std::cerr << "Runtime error: " << e.what() << "\n";
        m_Running = false;
    }
    catch(const std::bad_alloc& e){
        std::cerr << "Allocation error: " << e.what() << "\n";
        m_Running = false;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << "\n";
        m_Running = false;
    } 
    catch (...) {
        std::cerr << "Unknown exception caught \n";
        m_Running = false;
    }
}

void application::ConsoleTM::SetMessage(const std::string& m) noexcept{
    // not much we can do if this throws 
    // if it does we send the info to the console
    try{
        // prevent concurrent access to m_MessageQueue
        std::lock_guard<std::mutex> local_lock(m_Message_mtx);

        // add m to the queue
        m_MessageQueue.emplace(m);
    }
    catch(const std::runtime_error& e){
        std::cerr << "Runtime error: " << e.what() << "\n";
    }
    catch(const std::bad_alloc& e){
        std::cerr << "Allocation error: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << "\n";
    } 
    catch (...) {
        std::cerr << "Unknown exception caught \n";
    }
}

void application::ConsoleTM::RunMessages() noexcept{
    while(m_Running){
        to_console();
    }
}

void application::ConsoleTM::ReleaseBuffer() noexcept{
    m_release = true;
}

void application::ConsoleTM::end() noexcept{
    // if this throws not much we can do except set m_Running 
    // to false to exit the while loop in RunMessages() function.
    try{
        // process m_MessageQueue
        ReleaseBuffer();

        // create a unique lock mechanism so we can cause the thread to wait
        m_main_thread_lock = std::unique_lock<std::mutex>(m_main_thread_guard);

        // cause the thread to wait for m_MessageQueue to finish processing the queue
        m_main_thread_cv.wait(m_main_thread_lock, [this] {return !m_release; });
        
        // exit the while loop in RunMessages() function
        m_Running = false;
    }
    catch(const std::runtime_error& e){
        std::cerr << "Runtime error: " << e.what() << "\n";
        m_Running = false;
    }
    catch(const std::bad_alloc& e){
        std::cerr << "Allocation error: " << e.what() << "\n";
        m_Running = false;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << "\n";
        m_Running = false;
    } 
    catch (...) {
        std::cerr << "Unknown exception caught \n";
        m_Running = false;
    }
}


/////////////////////////////////////////////////
/* wide string version of ConsoleTM definitions*/
/////////////////////////////////////////////////

void application::wConsoleTM::to_console() noexcept{
    // << operator can throw
    // sleep_for() can throw
    // if they do we exit the to_console() function (any exception) and exit the while loop
    // in RunMessages() by setting m_Running to false
    try{
        // if m_release is true then m_MessageQueue gets displayed to the console
        // else the animation is displayed
        if(m_release){
            // prevent concurrent access to m_MessageQueue
            std::lock_guard<std::mutex> local_lock(m_Message_mtx);
            
            // process the queue until its empty
            while(!m_MessageQueue.empty()){
                // display the top message to the console
                std::wcout << m_MessageQueue.front() << "\n";

                // remove the top message
                m_MessageQueue.pop();
            }

            // m_MessageQueue is now empty
            // so reset m_release to false
            m_release = false;

            // notify the main thread if its waiting that processing has completed
            // and its safe to exit
            m_main_thread_cv.notify_one();
        }
        else{
            // ("/r")carriage return the animation so it displays correctly
            std::wcout << L"\r" <<  m_AnimationChars[m_AnimationIndex++];

            // cycle the index from 0 to 4
            m_AnimationIndex %= 4;

            // pause execution for 150ms so the output animation is fluid
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }        
    }
    catch(const std::runtime_error& e){
        std::cerr << "Runtime error: " << e.what() << "\n";
        m_Running = false;
    }
    catch(const std::bad_alloc& e){
        std::cerr << "Allocation error: " << e.what() << "\n";
        m_Running = false;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << "\n";
        m_Running = false;
    } 
    catch (...) {
        std::cerr << "Unknown exception caught \n";
        m_Running = false;
    }
}

void application::wConsoleTM::SetMessage(const std::wstring& m) noexcept{
    // not much we can do if this throws 
    // if it does we send the info to the console
    try{
        // prevent concurrent access to m_MessageQueue
        std::lock_guard<std::mutex> local_lock(m_Message_mtx);

        // add m to the queue
        m_MessageQueue.emplace(m);
    }
    catch(const std::runtime_error& e){
        std::cerr << "Runtime error: " << e.what() << "\n";
    }
    catch(const std::bad_alloc& e){
        std::cerr << "Allocation error: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << "\n";
    } 
    catch (...) {
        std::cerr << "Unknown exception caught \n";
    }
}

void application::wConsoleTM::RunMessages() noexcept{
    while(m_Running){
        to_console();
    }
}

void application::wConsoleTM::ReleaseBuffer() noexcept{
    m_release = true;
}

void application::wConsoleTM::end() noexcept{
    // if this throws not much we can do except set m_Running 
    // to false to exit the while loop in RunMessages() function.
    try{
        // process m_MessageQueue
        ReleaseBuffer();

        // create a unique lock mechanism so we can cause the thread to wait
        m_main_thread_lock = std::unique_lock<std::mutex>(m_main_thread_guard);

        // cause the thread to wait for m_MessageQueue to finish processing the queue
        m_main_thread_cv.wait(m_main_thread_lock, [this] {return !m_release; });
        
        // exit the while loop in RunMessages() function
        m_Running = false;
    }
    catch(const std::runtime_error& e){
        std::cerr << "Runtime error: " << e.what() << "\n";
        m_Running = false;
    }
    catch(const std::bad_alloc& e){
        std::cerr << "Allocation error: " << e.what() << "\n";
        m_Running = false;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << "\n";
        m_Running = false;
    } 
    catch (...) {
        std::cerr << "Unknown exception caught \n";
        m_Running = false;
    }
}