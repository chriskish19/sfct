#pragma once
#include "appMacros.hpp"


/////////////////////////////////////////////////////////////////////////////
/* This class will be OS specfic for more control over the copying process */
/* and it will be faster than FileCopy                                     */
/////////////////////////////////////////////////////////////////////////////

namespace application{
#if WINDOWS_BUILD
#include <Windows.h>

    class FastFileCopy{
        
    };


#endif
}