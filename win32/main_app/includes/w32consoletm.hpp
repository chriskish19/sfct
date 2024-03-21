#pragma once
#include "w32cpplib.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////////
// This header handles messages sent to the console.                                            //
// It uses a queue to prevent slow downs in the program when alot of messages are processed.    //
// It is designed to be launched on a thread given RunMessages().                               //
// Use SetMessage(const std::string& m) to add a message to the queue.                          //
// To display the queued messages call ReleaseBuffer().                                         //
//////////////////////////////////////////////////////////////////////////////////////////////////

namespace application{
    /// @brief this class will handle update messages to the console using a seperate thread
    /// ConsoleThreadedMessages
    class ConsoleTM{
    public:
        /// @brief default constructor
        ConsoleTM() = default;

        /// @brief default destructor
        ~ConsoleTM()= default;
        
        /// @brief delete the Copy constructor
        ConsoleTM(const ConsoleTM&) = delete;

        /// @brief delete the Copy assignment operator
        ConsoleTM& operator=(const ConsoleTM&) = delete;

        /// @brief delete the Move constructor
        ConsoleTM(ConsoleTM&&) = delete;

        /// @brief delete the Move assignment operator
        ConsoleTM& operator=(ConsoleTM&&) = delete;   

        /// @brief give this function to a TM object and call do_work()
        /// to_console() to run in a loop
        void RunMessages() noexcept;

        /// @brief adds m to the message queue
        /// @param m a message
        void SetMessage(const std::string& m) noexcept;

        /// @brief causes the queue the output all messages
        void ReleaseBuffer() noexcept;

        /// @brief ends the message stream
        void end() noexcept;
    private:
        /// @brief output m_Message to the console
        void to_console() noexcept;
        
        /// @brief prevent concurrent access to m_Message
        std::mutex m_Message_mtx;

        /// @brief symbols to play in the console animation
        char m_AnimationChars[4] = {'/', '-', '\\', '|'};
        
        /// @brief used to cycle through m_AnimationChars array
        int m_AnimationIndex = 0;

        /// @brief a thread safe boolean to close the message loop if needed
        std::atomic<bool> m_Running{true};

        /// @brief queue of messages 
        std::queue<std::string> m_MessageQueue;

        /// @brief true releases the queue
        std::atomic<bool> m_release{false};

        /// @brief for releasing message queue when end() is called
        /// causes the thread to wait until the queue has finished processing.
        std::mutex m_main_thread_guard;
		std::unique_lock<std::mutex> m_main_thread_lock;
		std::condition_variable m_main_thread_cv;
    };


    /// @brief wide string version of ConsoleTM class
    class wConsoleTM{
    public:
        /// @brief default constructor
        wConsoleTM() = default;

        /// @brief default destructor
        ~wConsoleTM()= default;
        
        /// @brief delete the Copy constructor
        wConsoleTM(const wConsoleTM&) = delete;

        /// @brief delete the Copy assignment operator
        wConsoleTM& operator=(const wConsoleTM&) = delete;

        /// @brief delete the Move constructor
        wConsoleTM(wConsoleTM&&) = delete;

        /// @brief delete the Move assignment operator
        wConsoleTM& operator=(wConsoleTM&&) = delete;

        /// @brief give this function to a TM object and call do_work()
        /// to_console() to run in a loop
        void RunMessages() noexcept;

        /// @brief adds m to the message queue
        /// @param m a message
        void SetMessage(const std::wstring& m) noexcept;

        /// @brief causes the queue the output all messages
        void ReleaseBuffer() noexcept;

        /// @brief ends the message stream
        void end() noexcept;
    private:
        /// @brief output m_Message to the console
        void to_console() noexcept;
        
        /// @brief prevent concurrent access to m_MessageQueue
        std::mutex m_Message_mtx;

        /// @brief symbols to play in the console animation
        wchar_t m_AnimationChars[4] = {L'/', L'-', L'\\', L'|'};
        
        /// @brief used to cycle through m_AnimationChars array
        int m_AnimationIndex = 0;

        /// @brief a thread safe boolean to close the message loop if needed
        std::atomic<bool> m_Running{true};

        /// @brief queue of messages 
        std::queue<std::wstring> m_MessageQueue;

        /// @brief true releases the queue
        std::atomic<bool> m_release{false};

        /// @brief for releasing message queue when end() is called
        /// causes the thread to wait until the queue has finished processing.
        std::mutex m_main_thread_guard;
		std::unique_lock<std::mutex> m_main_thread_lock;
		std::condition_variable m_main_thread_cv;
    };
}