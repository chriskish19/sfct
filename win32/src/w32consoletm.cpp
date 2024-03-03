#include "w32consoletm.hpp"


//////////////////////////////////////////////////////////
/* narrow string version of ConsoleTM class definitions */
//////////////////////////////////////////////////////////

void application::ConsoleTM::to_console() noexcept{
    if(m_release){
    
        try{
            std::lock_guard<std::mutex> local_lock(m_Message_mtx);
            while(!m_MessageQueue.empty()){
                std::cout << m_MessageQueue.front() << "\n";

                m_MessageQueue.pop();
            }
            m_release = false;
            m_main_thread_cv.notify_one();
        }
        catch (const std::filesystem::filesystem_error& e) {
            // Handle filesystem related errors
            std::cerr << "Filesystem error: " << e.what() << "\n";

            return;
        }
        catch(const std::runtime_error& e){
            // the error message
            std::cerr << "Runtime error: " << e.what() << "\n";

            return;
        }
        catch(const std::bad_alloc& e){
            // the error message
            std::cerr << "Allocation error: " << e.what() << "\n";

            return;
        }
        catch (const std::exception& e) {
            // Catch other standard exceptions
            std::cerr << "Standard exception: " << e.what() << "\n";

            return;
        } catch (...) {
            // Catch any other exceptions
            std::cerr << "Unknown exception caught \n";

            return;
        }
    }
    else{
        std::cout << "\r" <<  m_AnimationChars[m_AnimationIndex++];

        // cycle the index from 0 to 4
        m_AnimationIndex %= 4;

        // pause execution for 150ms so the output animation is fluid
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

void application::ConsoleTM::SetMessage(const std::string& m) noexcept{
    try{
        std::lock_guard<std::mutex> local_lock(m_Message_mtx);
        m_MessageQueue.emplace(m);
    }
    catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem related errors
        std::cerr << "Filesystem error: " << e.what() << "\n";
    }
    catch(const std::runtime_error& e){
        // the error message
        std::cerr << "Runtime error: " << e.what() << "\n";
    }
    catch(const std::bad_alloc& e){
        // the error message
        std::cerr << "Allocation error: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        // Catch other standard exceptions
        std::cerr << "Standard exception: " << e.what() << "\n";
    } catch (...) {
        // Catch any other exceptions
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
    try{
        ReleaseBuffer();
        m_main_thread_lock = std::unique_lock<std::mutex>(m_main_thread_guard);
        m_main_thread_cv.wait(m_main_thread_lock, [this] {return !m_release; });

        m_Running = false;
    }
    catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem related errors
        std::cerr << "Filesystem error: " << e.what() << "\n";
    }
    catch(const std::runtime_error& e){
        // the error message
        std::cerr << "Runtime error: " << e.what() << "\n";
    }
    catch(const std::bad_alloc& e){
        // the error message
        std::cerr << "Allocation error: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        // Catch other standard exceptions
        std::cerr << "Standard exception: " << e.what() << "\n";
    } catch (...) {
        // Catch any other exceptions
        std::cerr << "Unknown exception caught \n";
    }
}


/////////////////////////////////////////////////
/* wide string version of ConsoleTM definitions*/
/////////////////////////////////////////////////

void application::wConsoleTM::to_console() noexcept{
    if(m_release){
        
        try{
            std::lock_guard<std::mutex> local_lock(m_Message_mtx);
            while(!m_MessageQueue.empty()){
                std::wcout << m_MessageQueue.front() << "\n";

                m_MessageQueue.pop();
            }
            m_release = false;
            m_main_thread_cv.notify_one();
        }
        catch (const std::filesystem::filesystem_error& e) {
            // Handle filesystem related errors
            std::cerr << "Filesystem error: " << e.what() << "\n";

            return;
        }
        catch(const std::runtime_error& e){
            // the error message
            std::cerr << "Runtime error: " << e.what() << "\n";

            return;
        }
        catch(const std::bad_alloc& e){
            // the error message
            std::cerr << "Allocation error: " << e.what() << "\n";

            return;
        }
        catch (const std::exception& e) {
            // Catch other standard exceptions
            std::cerr << "Standard exception: " << e.what() << "\n";

            return;
        } catch (...) {
            // Catch any other exceptions
            std::cerr << "Unknown exception caught \n";

            return;
        }
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

void application::wConsoleTM::SetMessage(const std::wstring& m) noexcept{
    try{
        std::lock_guard<std::mutex> local_lock(m_Message_mtx);
        m_MessageQueue.emplace(m);
    }
    catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem related errors
        std::cerr << "Filesystem error: " << e.what() << "\n";
    }
    catch(const std::runtime_error& e){
        // the error message
        std::cerr << "Runtime error: " << e.what() << "\n";
    }
    catch(const std::bad_alloc& e){
        // the error message
        std::cerr << "Allocation error: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        // Catch other standard exceptions
        std::cerr << "Standard exception: " << e.what() << "\n";
    } catch (...) {
        // Catch any other exceptions
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
    try{
        ReleaseBuffer();
        m_main_thread_lock = std::unique_lock<std::mutex>(m_main_thread_guard);
        m_main_thread_cv.wait(m_main_thread_lock, [this] {return !m_release; });

        m_Running = false;
    }
    catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem related errors
        std::cerr << "Filesystem error: " << e.what() << "\n";
    }
    catch(const std::runtime_error& e){
        // the error message
        std::cerr << "Runtime error: " << e.what() << "\n";
    }
    catch(const std::bad_alloc& e){
        // the error message
        std::cerr << "Allocation error: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        // Catch other standard exceptions
        std::cerr << "Standard exception: " << e.what() << "\n";
    } catch (...) {
        // Catch any other exceptions
        std::cerr << "Unknown exception caught \n";
    }
}