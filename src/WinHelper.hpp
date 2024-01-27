#pragma once
#include "logger.hpp"
#include <fcntl.h>
#include <io.h>
#include "obj.hpp"
#include <vector>
#include "TM.hpp"
#include <optional>
#include <unordered_map>

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

    inline bool FastCopy(LPCWSTR srcPath, LPCWSTR destPath) {
        HANDLE hSource = CreateFile(srcPath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        HANDLE hDest = CreateFile(destPath, GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (hSource == INVALID_HANDLE_VALUE || hDest == INVALID_HANDLE_VALUE) {
            if (hSource != INVALID_HANDLE_VALUE) CloseHandle(hSource);
            if (hDest != INVALID_HANDLE_VALUE) CloseHandle(hDest);
            application::logger log(application::Error::DEBUG);
            log.to_console();
            log.to_log_file();
            log.to_output();
            return false;
        }

        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(hSource, &fileSize)) {
            application::logger log(application::Error::DEBUG);
            log.to_console();
            log.to_log_file();
            log.to_output();
            CloseHandle(hSource);
            CloseHandle(hDest);
            return false;
        }

        // Set file pointer to the desired size
        
        if (!SetFilePointerEx(hDest, fileSize, NULL, FILE_BEGIN)) {
            application::logger log(application::Error::DEBUG);
            log.to_console();
            log.to_log_file();
            log.to_output();
        }

        // Set the end of file
        if (!SetEndOfFile(hDest)) {
            application::logger log(application::Error::DEBUG);
            log.to_console();
            log.to_log_file();
            log.to_output();
        }

        HANDLE hSourceMapping = CreateFileMapping(hSource, nullptr, PAGE_READONLY, 0, 0, nullptr);
        HANDLE hDestMapping = CreateFileMapping(hDest, nullptr, PAGE_READWRITE, 0, 0, nullptr);

        if (!hSourceMapping || !hDestMapping) {
            if (hSourceMapping) CloseHandle(hSourceMapping);
            if (hDestMapping) CloseHandle(hDestMapping);
            CloseHandle(hSource);
            CloseHandle(hDest);
            application::logger log(application::Error::DEBUG);
            log.to_console();
            log.to_log_file();
            log.to_output();
            return false;
        }

        LPVOID pSource = MapViewOfFile(hSourceMapping, FILE_MAP_READ, 0, 0, 0);
        LPVOID pDest = MapViewOfFile(hDestMapping, FILE_MAP_WRITE, 0, 0, 0);

        if (!pSource || !pDest) {
            if (pSource) UnmapViewOfFile(pSource);
            if (pDest) UnmapViewOfFile(pDest);
            CloseHandle(hSourceMapping);
            CloseHandle(hDestMapping);
            CloseHandle(hSource);
            CloseHandle(hDest);
            application::logger log(application::Error::DEBUG);
            log.to_console();
            log.to_log_file();
            log.to_output();
            return false;
        }

        memcpy(pDest, pSource, fileSize.QuadPart);

        UnmapViewOfFile(pSource);
        UnmapViewOfFile(pDest);
        CloseHandle(hSourceMapping);
        CloseHandle(hDestMapping);
        CloseHandle(hSource);
        CloseHandle(hDest);

        return true;
    }

    inline std::uintmax_t MTFastCopy(const application::copyto& dir){
        if((dir.commands & application::cs::recursive) != application::cs::none){
            std::uintmax_t totalsize{};
            std::vector<application::paths*> pPaths;
            application::TM workers;

            for(const auto& entry:std::filesystem::recursive_directory_iterator(dir.source)){
                totalsize += entry.file_size();
                const auto& path = entry.path();
                auto relativePath = std::filesystem::relative(path, dir.source);
                if (entry.is_directory()) {
                    std::filesystem::create_directories(dir.destination / relativePath);
                } else if (entry.is_regular_file() && entry.file_size() != 0) {
                    application::paths* _paths = new application::paths(path,dir.destination / relativePath);
                    pPaths.push_back(_paths);
                    workers.do_work(&Windows::FastCopy,_paths->m_src.c_str(),_paths->m_dst.c_str());
                    if(workers.join_one()){
                        application::paths* p_path = pPaths.front();
                        if(p_path) delete p_path;
                        pPaths.erase(pPaths.begin());
                    }
                }
            }
            workers.join_all();

            // cleanup
            for(const auto path:pPaths){
                if(path) delete path;
            }

            return totalsize;
        }
        else{
            std::uintmax_t totalsize{};
            std::vector<application::paths*> pPaths;
            application::TM workers;

            for(const auto& entry:std::filesystem::directory_iterator(dir.source)){
                totalsize += entry.file_size();
                const auto& path = entry.path();
                auto relativePath = std::filesystem::relative(path, dir.source);
                if (entry.is_directory()) {
                    std::filesystem::create_directories(dir.destination / relativePath);
                } else if (entry.is_regular_file() && entry.file_size() != 0) {
                    application::paths* _paths = new application::paths(path,dir.destination / relativePath);
                    pPaths.push_back(_paths);
                    workers.do_work(&Windows::FastCopy,_paths->m_src.c_str(),_paths->m_dst.c_str());
                    if(workers.join_one()){
                        application::paths* p_path = pPaths.front();
                        if(p_path) delete p_path;
                        pPaths.erase(pPaths.begin());
                    }
                }
            }
            workers.join_all();

            // cleanup
            for(const auto path:pPaths){
                if(path) delete path;
            }

            return totalsize;
        }
    }

    inline std::optional<std::shared_ptr<std::unordered_map<std::filesystem::path,std::filesystem::path>>> GetClipBoardFilePaths(){
        if (!OpenClipboard(nullptr)) {
            application::logger log(application::Error::WARNING);
            log.to_console();
            log.to_log_file();
            log.to_output();
            return std::nullopt;
        }

        HANDLE hData = GetClipboardData(CF_HDROP);
        if (hData == nullptr) {
            application::logger log(application::Error::INFO);
            log.to_log_file();
            CloseClipboard();
            return std::nullopt;
        }

        HDROP hDrop = static_cast<HDROP>(GlobalLock(hData));
        if (hDrop == nullptr) {
            application::logger log(application::Error::WARNING);
            log.to_console();
            log.to_log_file();
            log.to_output();
            CloseClipboard();
            return std::nullopt;
        }

        UINT nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);
        if(nFiles>0){
            std::shared_ptr<std::unordered_map<std::filesystem::path,std::filesystem::path>> pPaths_mp{std::make_shared<std::unordered_map<std::filesystem::path,std::filesystem::path>>()};
            for (UINT i = 0; i < nFiles; ++i) {
                // Get the required buffer size for each file path
                UINT bufferSize = DragQueryFile(hDrop, i, nullptr, 0) + 1; // +1 for null terminator

                // Allocate a buffer for the file path
                WCHAR* tmp_path = new WCHAR[bufferSize];

                // Retrieve the file path
                if (DragQueryFile(hDrop, i, tmp_path, bufferSize)) {
                    std::filesystem::path filepath(tmp_path);
                    pPaths_mp->emplace(filepath.filename(),filepath);
                }

                if(tmp_path) delete tmp_path;
            }
            GlobalUnlock(hData);
            DragFinish(hDrop);
            CloseClipboard();
            return pPaths_mp;
        }
        
        GlobalUnlock(hData);
        DragFinish(hDrop);
        CloseClipboard();
        return std::nullopt;
    } 

      
}
#endif