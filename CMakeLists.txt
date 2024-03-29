# Set the minimum required version of CMake to be 3.13
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Create a project named sfct
project(sfct VERSION 1.0.0.2)

# Set the variable CMAKE_CXX_STANDARD to c++20
# and the variable CMAKE_CXX_STANDARD_REQUIRED to True
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Add the source files here
set(SOURCE_FILES    src/main.cpp
                    src/TM.hpp
                    src/TM.cpp
                    src/Logger.hpp
                    src/Logger.cpp
                    src/FileParse.hpp
                    src/FileParse.cpp
                    src/DirectorySignal.hpp
                    src/DirectorySignal.cpp
                    src/ConsoleTM.hpp
                    src/ConsoleTM.cpp
                    src/ConsoleApp.hpp
                    src/ConsoleApp.cpp
                    src/appMacros.hpp
                    src/windows_helper.hpp
                    src/obj.hpp
                    src/args.hpp
                    src/benchmark.hpp
                    src/benchmark.cpp
                    src/constants.hpp
                    src/sfct_api.hpp
                    src/sfct_api.cpp
                    src/queue_system.hpp
                    src/timer.hpp
                    src/timer.cpp
                    src/directory_copy.hpp
                    src/directory_copy.cpp)


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


