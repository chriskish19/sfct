#pragma once
#include "logger.hpp"
#include <fcntl.h>
#include <io.h>

namespace Windows{
    inline void enableANSIEscapeCodes() {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE) {
            application::logger log(application::Error::WARNING,App_LOCATION);
            log.to_console();
            log.to_log_file();
            log.to_output();
            return;
        }

        DWORD dwMode = 0;
        if (!GetConsoleMode(hOut, &dwMode)) {
            application::logger log(application::Error::WARNING,App_LOCATION);
            log.to_console();
            log.to_log_file();
            log.to_output();
            return;
        }

        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if (!SetConsoleMode(hOut, dwMode)) {
            application::logger log(application::Error::WARNING,App_LOCATION);
            log.to_console();
            log.to_log_file();
            log.to_output();
        }

        // Set the console mode to handle wide characters
        _setmode(_fileno(stdout), _O_U16TEXT);
    }
}