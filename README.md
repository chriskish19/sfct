# Simple Folder Copy Tool
Keeps directories synced, by monitoring when files are added and then copies those files to a destination directory. Works similar to a cloud drive but locally.

# Build From Source Code
You will need installed on your system:
1. [Git](https://git-scm.com/download/win)
2. [CMake](https://cmake.org/)
3. IDE(Integrated Development Enviroment such as [Visual Studio 2022](https://visualstudio.microsoft.com/vs/community/))

In your terminal:
```powershell
# clone the repository
git clone https://github.com/chriskish19/sfct.git

# navigate to the directory
cd sfct

# Make a build folder
mkdir build
cd build

# To build your IDE solution
cmake ../

# Compile the project into an executable
cmake --build .
```

# Getting Started
## Get the latest release
Download the latest release from the releases page.

## Setup
Run sfct.exe for the first time and it will create an sfct_list.txt file in the current directory. This is where you specify the directories you want synced/copied.
The syntax is simple with only one command "copy" then the source directory using the keyword "src" and the destination directory using the keyword "dst" all in braces. 
Semi-colon on the end of each directory is needed aswell.

sftc_list.txt Example:
```
copy
{
    src C:/test example;
    dst D:/test example;
}

copy
{
    src D:\test\beattles;
    dst D:\dest dir1;
}

```

There is no limit to the number of directories, but each will use 10MB of memory for monitoring so watch that if your adding 100 directories it will use 1GB of memory.

Add your directories to the sfct_list.txt file, save it and re-run sfct.exe you should see a message in the console that says "Succesfully opened sfct_list.txt".

# Info
## Current Limitations
1. Single threaded copying(its still really fast at copying it maxes out my gen3 ssd +2GB/s)
2. ~~No control over copying flags~~
3. No GUI
4. Windows Only
5. ~~No control over syncing(currently updates existing files)~~


## Future Plans
1. ~~Add more control over syncing and copying(sftc_list.txt will have more commands and arguments)~~
2. Implement multithreaded copying
3. Linux support
4. Mac support
5. Add GUI

## Benchmarks
My laptop specs:
Intel i7 9750H 2.6ghz Cpu
64GB 2666mhz Ram
crucial p3 gen3 nvme ssd 4tb

## Unreal Engine source code benchmark
Average speed to fast copy UnrealEngine source code total size 27gb+ and 161,469 Files.
PS C:\Users\chris\projects\sfct\build\Debug> .\sfct
[INFO][2024-01-22 22:16:08.2847663] File: C:\Users\chris\projects\sfct\src\FileParse.cpp Line: 51 Function: bool __cdecl application::FileParse::OpenFile(void) Message: Succesfully opened sfct_list.txt
Preparing to fast copy files
-Speed in MB/s: 290.021765

Average speed to fast copy Battlefield 1 game files total size 
PS C:\Users\chris\projects\sfct\build\Debug> .\sfct
[INFO][2024-01-22 22:40:42.0452485] File: C:\Users\chris\projects\sfct\src\FileParse.cpp Line: 51 Function: bool __cdecl application::FileParse::OpenFile(void) Message: Succesfully opened sfct_list.txt
Preparing to fast copy files
