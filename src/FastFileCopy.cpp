#include "FastFileCopy.hpp"

#if WINDOWS_BUILD
application::FastFileCopy::FastFileCopy(std::shared_ptr<std::vector<copyto>> dirs)
:m_dirs(dirs)
{
    if(!dirs){
        logger log(App_MESSAGE("nullptr"),Error::FATAL);
        log.to_console();
        log.to_log_file();
        throw std::runtime_error("");
    }
}

void application::FastFileCopy::copy()
{
    for(const auto& dir:*m_dirs){
        std::uintmax_t totalsize{};
        benchmark speed;
        speed.start_clock();
        totalsize = Windows::MTFastCopy(dir);
        speed.end_clock();
        double rate = speed.speed(totalsize);
        m_MessageStream.SetMessage(STRING(dir.source) + App_MESSAGE("- Speed in MB/s: ") + TOSTRING(rate));
        m_MessageStream.ReleaseBuffer();
    }
}
#endif