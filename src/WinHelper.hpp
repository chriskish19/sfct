#pragma once
#include "logger.hpp"
#include <fcntl.h>
#include <io.h>

/////////////////////////////////////////////////////////////////////////////////
// This header contains windows specific functions
/////////////////////////////////////////////////////////////////////////////////


#if WINDOWS_BUILD
namespace Windows{
    inline void enableANSIEscapeCodes() {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE) {
            application::logger log(application::Error::WARNING);
            log.to_console();
            log.to_log_file();
            log.to_output();
            return;
        }

        DWORD dwMode = 0;
        if (!GetConsoleMode(hOut, &dwMode)) {
            application::logger log(application::Error::WARNING);
            log.to_console();
            log.to_log_file();
            log.to_output();
            return;
        }

        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if (!SetConsoleMode(hOut, dwMode)) {
            application::logger log(application::Error::WARNING);
            log.to_console();
            log.to_log_file();
            log.to_output();
        }
    }

    // Set the console mode to handle wide characters UTF-16
    inline void SetWideConsoleMode(){
        _setmode(_fileno(stdout), _O_U16TEXT);          
    }

    inline bool FastCopy(LPCWSTR srcPath, LPCWSTR destPath, const DWORD chunkSize = 64 * 1024 * 1024) { // Default chunk size: 64 MB
        HANDLE hSource = CreateFile(srcPath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        HANDLE hDest = CreateFile(destPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (hSource == INVALID_HANDLE_VALUE || hDest == INVALID_HANDLE_VALUE) {
            CloseHandle(hSource);
            CloseHandle(hDest);
            return false;
        }

        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(hSource, &fileSize)) {
            CloseHandle(hSource);
            CloseHandle(hDest);
            return false;
        }

        LARGE_INTEGER offset = {0};

        HANDLE hSourceMapping = CreateFileMapping(hSource, nullptr, PAGE_READONLY, 0, 0, nullptr);
        HANDLE hDestMapping = CreateFileMapping(hDest, nullptr, PAGE_READWRITE, 0, 0, nullptr);

        if (!hSourceMapping || !hDestMapping) {
            CloseHandle(hSourceMapping);
            CloseHandle(hDestMapping);
            return false;
        }

        while (offset.QuadPart < fileSize.QuadPart) {
            DWORD bytesToMap = (DWORD)min((LONGLONG)chunkSize, fileSize.QuadPart - offset.QuadPart);

            LPVOID pSource = MapViewOfFile(hSourceMapping, FILE_MAP_READ, offset.HighPart, offset.LowPart, bytesToMap);
            LPVOID pDest = MapViewOfFile(hDestMapping, FILE_MAP_WRITE, offset.HighPart, offset.LowPart, bytesToMap);

            if (!pSource || !pDest) {
                UnmapViewOfFile(pSource);
                UnmapViewOfFile(pDest);
                break;
            }

            memcpy(pDest, pSource, bytesToMap);

            UnmapViewOfFile(pSource);
            UnmapViewOfFile(pDest);

            offset.QuadPart += bytesToMap;
        }

        CloseHandle(hSourceMapping);
        CloseHandle(hDestMapping);
        CloseHandle(hSource);
        CloseHandle(hDest);

        return offset.QuadPart == fileSize.QuadPart;
    }
}
#endif