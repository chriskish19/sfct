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
Run sfct.exe for the first time and it will create an sfct_lists.txt file in the current directory. This is where you specify the directories you want synced/copied.
The syntax is simple with only one command "copy" then the source directory using the keyword "src" and the destination directory using the keyword "dst" all in braces. 
Semi-colon on the end of each directory is needed aswell.

sftc_lists.txt Example:
```
# valid entries
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

Add your directories to the sfct_lists.txt file, save it and re-run sfct.exe you should see a message in the console that says "Succesfully opened sfct_lists.txt".

# Info
## Current Limitations
1. Single threaded copying(its still really fast at copying it maxes out my gen3 ssd +2GB/s)
2. No control over copying flags
3. No GUI
4. Windows Only
5. No control over syncing(currently updates existing files)


## Future Plans
1. Add more control over syncing and copying(sftc_lists.txt will have more commands and arguments)
2. Implement multithreaded copying
3. Linux support
4. Mac support
5. Add GUI

## Get the beta version of sfct
There is a dev branch I work on daily and its usually compile ready and tested. But not always. 
To try out the dev branch:

```powershell
# clone the repository
git clone https://github.com/chriskish19/sfct.git

# navigate to the directory
cd sfct

# Make a build folder
mkdir build
cd build

# check which branch is currently selected
git branch

# switch to the dev branch
git checkout dev

# To build your IDE solution
cmake ../

# Compile the project into an executable
cmake --build .
```
