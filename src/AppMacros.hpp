#pragma once

#ifndef WINDOWS_BUILD
#define WINDOWS_BUILD _WIN32
#endif

#ifndef LINUX_BUILD
#define LINUX_BUILD __linux__
#endif

#ifndef MAC_BUILD
#define MAC_BUILD __APPLE__
#endif