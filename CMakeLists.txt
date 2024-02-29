# Set the minimum required version of CMake to be 3.13
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Create a project named sfct
project(sfct VERSION 1.0.0.2)

# Set the variable CMAKE_CXX_STANDARD to c++20
# and the variable CMAKE_CXX_STANDARD_REQUIRED to True
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)



# makes for simple include paths in headers
include_directories(src/logger)
include_directories(src/opengl)


# Add the source files here
set(SOURCE_FILES    src/app/main.cpp
                    src/app/TM.hpp
                    src/app/TM.cpp
                    src/logger/Logger.hpp
                    src/logger/Logger.cpp
                    src/app/FileParse.hpp
                    src/app/FileParse.cpp
                    src/app/DirectorySignal.hpp
                    src/app/DirectorySignal.cpp
                    src/app/ConsoleTM.hpp
                    src/app/ConsoleTM.cpp
                    src/app/ConsoleApp.hpp
                    src/app/ConsoleApp.cpp
                    src/app/appMacros.hpp
                    src/app/windows_helper.hpp
                    src/app/obj.hpp
                    src/app/args.hpp
                    src/app/benchmark.hpp
                    src/app/benchmark.cpp
                    src/app/constants.hpp
                    src/app/sfct_api.hpp
                    src/app/sfct_api.cpp
                    src/app/queue_system.hpp
                    src/app/timer.hpp
                    src/app/timer.cpp
                    src/app/directory_copy.hpp
                    src/app/directory_copy.cpp)


add_executable(sfct ${SOURCE_FILES})


target_link_options(sfct PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/SUBSYSTEM:CONSOLE>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wl,/subsystem:console>
)


# Check if the build is for Windows
if(WIN32)
    # Define UNICODE for Windows builds
    add_definitions(-DUNICODE -D_UNICODE)

    # link the manifest to the project
    target_sources(sfct PRIVATE win32/sfct.exe.manifest)
endif()


