# Set the minimum required version of CMake to be 3.13
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Create a project named sfct
project(sfct VERSION 1.0.0.1)

# Set the variable CMAKE_CXX_STANDARD to c++20
# and the variable CMAKE_CXX_STANDARD_REQUIRED to True
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Conditionally add the OS specific source files
# windows
if(WIN32)
    # Add the win32 source files here
    set(SOURCE_FILES    win32/src/w32main.cpp
                        win32/src/w32tm.cpp
                        win32/src/w32logger.cpp
                        win32/src/w32fileparse.cpp
                        win32/src/w32directorysignal.cpp
                        win32/src/w32consoletm.cpp
                        win32/src/w32consoleapp.cpp
                        win32/src/w32benchmark.cpp
                        win32/src/w32sfct_api.cpp
                        win32/src/w32timer.cpp
                        win32/src/w32directorycopy.cpp
                        win32/src/w32specific_api.cpp)

# if linux
elseif(UNIX AND NOT APPLE)
    # Add the linux src files here
    set(SOURCE_FILES    linux/src/lxspecific.cpp )



endif()


add_executable(${PROJECT_NAME} ${SOURCE_FILES})


# if linux use opengl
if(UNIX AND NOT APPLE)
    # OpenGL package is required
    find_package(OpenGL REQUIRED)

    # Include directories for GLEW and GLFW using target_include_directories
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/linux/opengl/glew
        ${CMAKE_SOURCE_DIR}/linux/opengl/glfw
    )

    # Link against GLEW, GLFW, and OpenGL libraries
    # NOTE: remember to create linux compiled versions or this will error
    target_link_libraries(${PROJECT_NAME} PRIVATE
    debug ${CMAKE_SOURCE_DIR}/linux/opengl/glew/# add linux version here # this is x64 debug version
    optimized ${CMAKE_SOURCE_DIR}/linux/opengl/glew/# add linux version here # this is x64 release version
    
    optimized ${CMAKE_SOURCE_DIR}/linux/opengl/glfw/# add linux version here # this is x64 release version
    debug ${CMAKE_SOURCE_DIR}/linux/opengl/glfw/# add linux version here # this is x64 debug version
        OpenGL::GL
    )

    # Path to the glew DLL file
    set(GLEW_DLL_PATH_DEBUG ${CMAKE_SOURCE_DIR}/linux/opengl/glew/) # add linux version here
    set(GLEW_DLL_PATH_RELEASE ${CMAKE_SOURCE_DIR}/linux/opengl/glew/) # add linux version here

    # Post-build command to copy GLEW DLL
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<$<CONFIG:Debug>:${GLEW_DLL_PATH_DEBUG}>$<$<CONFIG:Release>:${GLEW_DLL_PATH_RELEASE}>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

# if windows
elseif(WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/SUBSYSTEM:CONSOLE>
    )

    # Define UNICODE for Windows builds
    add_definitions(-DUNICODE -D_UNICODE)

    # link the manifest to the project
    target_sources(${PROJECT_NAME} PRIVATE win32/manifest/sfct.exe.manifest)


endif()




