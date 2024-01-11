#pragma once
#include <string>
#include <thread>
#include <memory>
#include <mutex>
#include <format>
#include <iostream>
#include <atomic>
#include "AppMacros.hpp"
#include <syncstream>
#include <queue>

namespace application{
    // this class will handle update messages to the console using a seperate thread
    // ConsoleThreadedMessages
    class ConsoleTM{
    public:
        // give this function to a TM object and call do_work()
        // to_console() to run in a loop
        void RunMessages();

        // changes m_Message to m
        void SetMessage(const std::string& m);

        // get a shared_ptr to m_Running if needed else where in the program
        const std::shared_ptr<std::atomic<bool>> GetSPRunning(){return m_Running;}

        void ReleaseBuffer();
    private:
        // output m_Message to the console
        void to_console();
        
        // prevent concurrent access to m_Message
        std::mutex m_Message_mtx;

        // symbols to play in the console animation
        char m_AnimationChars[4] = {'/', '-', '\\', '|'};
        int m_AnimationIndex = 0;

        // a thread safe boolean shared_ptr to close the message loop if needed
        std::shared_ptr<std::atomic<bool>> m_Running{std::make_shared<std::atomic<bool>>(true)};

        std::queue<std::string> m_MessageQueue;

        bool m_release{false};
    };


    // wide string version of ConsoleTM class
    class wConsoleTM{
    public:
        // give this function to a TM object and call do_work()
        // to_console() to run in a loop
        void RunMessages();

        // adds a message to the queue
        void SetMessage(const std::wstring& m);

        // get a shared_ptr to m_Running if needed else where in the program
        const std::shared_ptr<std::atomic<bool>> GetSPRunning(){return m_Running;}

        
        void ReleaseBuffer();
    private:
        // output m_Message to the console
        void to_console();
        
        // prevent concurrent access to m_MessageQueue
        std::mutex m_Message_mtx;

        // symbols to play in the console animation
        wchar_t m_AnimationChars[4] = {L'/', L'-', L'\\', L'|'};
        int m_AnimationIndex = 0;

        // a thread safe boolean shared_ptr to close the message loop if needed
        std::shared_ptr<std::atomic<bool>> m_Running{std::make_shared<std::atomic<bool>>(true)};

        std::queue<std::wstring> m_MessageQueue;

        bool m_release{false};
    };
}