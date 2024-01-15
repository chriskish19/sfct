#pragma once

///////////////////////////////////////////////////////////////
// This header defines and uses macros depending on the OS
///////////////////////////////////////////////////////////////


#ifdef _WIN32
#define WINDOWS_BUILD 1
#define STRING std::wstring
#define OFSTREAM std::wofstream
#define CONSOLETM wConsoleTM
#else
#define WINDOWS_BUILD 0
#endif
#ifdef __linux__
#define LINUX_BUILD 1
#define STRING std::string
#define OFSTREAM std::ofstream
#define CONSOLETM ConsoleTM
#else
#define LINUX_BUILD 0
#endif

#ifdef __APPLE__
#define MAC_BUILD 1
#define STRING std::string
#define OFSTREAM std::ofstream
#else
#define MAC_BUILD 0
#endif
