# Set the minimum required version of CMake to be 3.13
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Create a project named sfct
project(sfct VERSION 1.0.0.1)

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


add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# OpenGL package is required for GUI
find_package(OpenGL REQUIRED)

# Include directories for GLEW and GLFW using target_include_directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/opengl/glew
    ${CMAKE_SOURCE_DIR}/src/opengl/glfw
)

# Link against GLEW, GLFW, and OpenGL libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/opengl/glew/glew32.lib # this is x64 version release
    ${CMAKE_SOURCE_DIR}/src/opengl/glfw/glfw3.lib # this is x64 version release
    OpenGL::GL
)


# Check if the build is for Windows
if(WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/SUBSYSTEM:CONSOLE>
    )

    # Define UNICODE for Windows builds
    add_definitions(-DUNICODE -D_UNICODE)

    # link the manifest to the project
    target_sources(${PROJECT_NAME} PRIVATE win32/sfct.exe.manifest)
endif()

# Path to the glew DLL file
set(GLEW_DLL_PATH ${CMAKE_SOURCE_DIR}/src/opengl/glew/glew32.dll)


# Post-build command to copy GLEW DLL
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${GLEW_DLL_PATH}
    $<TARGET_FILE_DIR:${PROJECT_NAME}>)