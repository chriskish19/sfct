#pragma once
#include <string>
#include "w32consoletm.hpp"
#include <fstream>

#define TOSTRING std::to_wstring
#define STDOUT std::wcout
#define App_MESSAGE(x) L##x 

namespace application{
    using STRING = std::wstring;
    using OFSTREAM = std::wofstream;
    using CONSOLETM = wConsoleTM;
}

namespace sfct_api{
    using STRING = std::wstring;
    using OFSTREAM = std::wofstream;
    using CONSOLETM = application::wConsoleTM;
}
