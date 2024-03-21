#pragma once
#include "w32cpplib.hpp"
#include "w32consoletm.hpp"


#define TOSTRING std::to_wstring                // macro for converting a number to a wide string
#define STDOUT std::wcout                       // macro for std::wcout, this is useful because I can change it to std::cout if needed
#define App_MESSAGE(x) L##x                     // macro that prepends an L to a string literal to make it a wide string literal



// using is a better option than macros
// they are confined to a namespace scope
namespace application{
    using STRING = std::wstring;                // STRING is a wide string in the application namespace 
    using OFSTREAM = std::wofstream;            // OFSTREAM is a wide file stream in the application namespace
    using CONSOLETM = wConsoleTM;               // CONSOLETM is the wide string version of ConsoleTM class
}

namespace sfct_api{
    using STRING = std::wstring;                    // STRING is a wide string in the sfct_api namespace
    using OFSTREAM = std::wofstream;                // OFSTREAM is a wide file stream in the sfct_api namespace
    using CONSOLETM = application::wConsoleTM;      // CONSOLETM is the wide string version of ConsoleTM class
}
