#include "w32specific_api.hpp"

bool Windows::FastCopy(const std::filesystem::path src, const std::filesystem::path dst) noexcept
{
    try{
        HANDLE hSource = CreateFile(src.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        HANDLE hDest = CreateFile(dst.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

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
    catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem related errors
        std::cerr << "Filesystem error: " << e.what() << "\n";

        return false;
    }
    catch(const std::runtime_error& e){
        // the error message
        std::cerr << "Runtime error :" << e.what() << "\n";
        
        return false;
    }
    catch(const std::bad_alloc& e){
        // the error message
        std::cerr << "Allocation error: " << e.what() << "\n";

        return false;
    }
    catch (const std::exception& e) {
        // Catch other standard exceptions
        std::cerr << "Standard exception: " << e.what() << "\n";

        return false;
    } catch (...) {
        // Catch any other exceptions
        std::cerr << "Unknown exception caught \n";

        return false;
    }
}

