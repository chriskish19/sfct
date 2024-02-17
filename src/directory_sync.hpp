#pragma once
#include <vector>
#include "obj.hpp"
#include <thread>
#include <condition_variable>
#include <atomic>
#include "sfct_api.hpp"
#include "queue_system.hpp"


namespace application{
    void directory_sync(const std::vector<application::copyto>& dirs, std::atomic<bool>* procceed,std::condition_variable* local_thread_cv,
                        std::atomic<bool>* running);
}