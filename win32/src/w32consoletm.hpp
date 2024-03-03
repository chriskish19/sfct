#pragma once
#include "w32cpplib.hpp"

//////////////////////////////////////////////////////////////////
// This header handles messages sent to the console.
// It uses a queue to prevent slow downs in the program when alot of messages are processed.
// It is designed to be launched on a thread given RunMessages().
// Use SetMessage(const std::string& m) to add a message to the queue.
// To display the queued messages call ReleaseBuffer().
//////////////////////////////////////////////////////////////////

namespace application{
    // this class will handle update messages to the console using a seperate thread
    // ConsoleThreadedMessages
    class ConsoleTM{
    public:
        // default constructor
        ConsoleTM() = default;

        // default destructor
        ~ConsoleTM()= default;
        
        // Copy constructor
        ConsoleTM(const ConsoleTM&) = delete;

        // Copy assignment operator
        ConsoleTM& operator=(const ConsoleTM&) = delete;

        // Move constructor
        ConsoleTM(ConsoleTM&&) = delete;

        // Move assignment operator
        ConsoleTM& operator=(ConsoleTM&&) = delete;   

        // give this function to a TM object and call do_work()
        // to_console() to run in a loop
        void RunMessages() noexcept;

        // changes m_Message to m
        void SetMessage(const std::string& m) noexcept;

        // causes the queue the output all messages
        void ReleaseBuffer() noexcept;

        // ends the message stream
        void end() noexcept;
    private:
        // output m_Message to the console
        void to_console() noexcept;
        
        // prevent concurrent access to m_Message
        std::mutex m_Message_mtx;

        // symbols to play in the console animation
        char m_AnimationChars[4] = {'/', '-', '\\', '|'};
        int m_AnimationIndex = 0;

        // a thread safe boolean to close the message loop if needed
        std::atomic<bool> m_Running{true};

        // queue of messages 
        std::queue<std::string> m_MessageQueue;

        // true releases the queue
        std::atomic<bool> m_release{false};

        // for releasing message queue when end() is called
        std::mutex m_main_thread_guard;
		std::unique_lock<std::mutex> m_main_thread_lock;
		std::condition_variable m_main_thread_cv;
    };


    // wide string version of ConsoleTM class
    class wConsoleTM{
    public:
        // default constructor
        wConsoleTM() = default;

        // default destructor
        ~wConsoleTM()= default;
        
        // Copy constructor
        wConsoleTM(const wConsoleTM&) = delete;

        // Copy assignment operator
        wConsoleTM& operator=(const wConsoleTM&) = delete;

        // Move constructor
        wConsoleTM(wConsoleTM&&) = delete;

        // Move assignment operator
        wConsoleTM& operator=(wConsoleTM&&) = delete;

        // give this function to a TM object and call do_work()
        // to_console() to run in a loop
        void RunMessages() noexcept;

        // adds a message to the queue
        void SetMessage(const std::wstring& m) noexcept;

        // causes the queue the output all messages
        void ReleaseBuffer() noexcept;

        // ends the message stream
        void end() noexcept;
    private:
        // output m_Message to the console
        void to_console() noexcept;
        
        // prevent concurrent access to m_MessageQueue
        std::mutex m_Message_mtx;

        // symbols to play in the console animation
        wchar_t m_AnimationChars[4] = {L'/', L'-', L'\\', L'|'};
        int m_AnimationIndex = 0;

        // a thread safe boolean shared_ptr to close the message loop if needed
        std::atomic<bool> m_Running{true};

        // queue of messages 
        std::queue<std::wstring> m_MessageQueue;

        // true releases the queue
        std::atomic<bool> m_release{false};

        // for releasing message queue when end() is called
        std::mutex m_main_thread_guard;
		std::unique_lock<std::mutex> m_main_thread_lock;
		std::condition_variable m_main_thread_cv;
    };
}