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
Run sfct.exe for the first time and it will create an sfct_list.txt file in the current directory. This is where you specify the directories you want synced, copied or benchmarked.

sftc_list.txt Example:
```
copy -recursive -update
{
    src C:/test example;
    dst D:/test example;
}

fast_copy -recursive -update
{
    src C:/test;
    dst D:/test;
}

benchmark -create -fast
{
    src C:/benchtest/src;
    dst C:/benchtest/dst;
}

monitor -recursive -update -sync
{
    src C:/test example;
    dst D:/test example;
}
```

There is no limit to the number of directories. Each monitored directory will use 10MB of memory for monitoring so watch that if your adding 100 directories to monitor it will use 1GB of memory.

Add your directories to the sfct_list.txt file, save it and re-run sfct.exe you should see a message in the console that says "Succesfully opened sfct_list.txt".

## Commands and Args
### copy
Checks that files are available and then copies the files using std::filesystem::copy.

### fast_copy
Does not check if the files are available. Performs a copy using OS specific fast copy function depending on the amount of files and their average size. Many small files are better handled by std::filesystem::copy.
If the average file size is above a threashold OS specific FastCopy is used and it is multithreaded. The amount of threads used is determined by the TM class, 8 is the max.

### monitor
Monitors a directory for changes, when changes occur the function wakes up and performs the arguments specified. Typically recursive, update, and sync.

### benchmark
Performs a speed test of the copy operation, currently std::filesystem::copy is used. When -4k arg is supplied a large number of small files are created and copied. If -create arg is supplied the directories will be created.

### -recursive
Sub-directories are included.

### -update
The existing file is checked and updated to the src version if it is newer.

### -overwrite
The existing file is replaced.

### -create
Creates src and dst directories.

### -sync
Syncs a src directory to a dst directory. When a file or directory is added to src it is added to dst and when a directory or file is removed from src, it is removed from dst.

### -sync_add
Syncs a src directory to a dst directory. When a file or directory is added to src it is also added to dst but when a file or directory is removed from src it is not removed from dst.

### -single
Sub-directories not included only the files and folders in the directory.

### -4k
Performs a benchmark to test copy speed using many small files.

### -fast
Uses the OS specific fast copy instead of std::filesystem::copy.

## Valid combinations of commands and args
### copy
copy -recursive -update<br>
copy -recursive -overwrite<br>
copy -single -update<br>
copy -single -overwrite<br>

### monitor
monitor -recursive -sync -update<br>
monitor -recursive -sync -overwrite<br>
monitor -single -sync -update<br>
monitor -single -sync -overwrite<br>
monitor -single -sync_add -update<br>
monitor -single -sync_add -overwrite<br>
monitor -recursive -sync_add -update<br>
monitor -recursive -sync_add -overwrite<br>

### fast_copy
fast_copy -recursive -update<br>
fast_copy -recursive -overwrite<br>
fast_copy -single -update<br>
fast_copy -single -overwrite<br>

### benchmark
benchmark -create -4k<br>
benchmark -create<br>
benchmark -4k<br>
benchmark<br>
benchmark -create -4k -fast<br>
benchmark -create -fast<br>
benchmark -4k -fast<br>
benchmark -fast<br>

# Info
## Current Limitations
1. ~~Single threaded copying(its still really fast at copying it maxes out my gen3 ssd +2GB/s)~~
2. ~~No control over copying flags~~
3. No GUI
4. Windows Only
5. ~~No control over syncing(currently updates existing files)~~
6. Lacks robust error handling which is why I only release debug versions.
7. Needs a terminal window to stay running.
8. Cannot copy a cloud drive folder, program crashes.
9. Checks if a file is available by pooling fstream.
10. No checks to avoid sub-directory recursive monitoring. Currently depends on the user not to double monitor.
11. Unpolished program still in the early stages of development. 

## Future Plans
1. ~~Add more control over syncing and copying(sftc_list.txt will have more commands and arguments)~~
2. ~~Implement multithreaded copying~~
3. Linux support
4. Mac support
5. Add GUI
6. Address current limitations.
7. Add more features.

## Benchmarks
My laptop specs:<br>
Intel i7 9750H 2.6ghz Cpu<br>
64GB 2666mhz Ram<br>
(2)crucial p3 gen3 nvme ssd 4tb<br>

### Unreal Engine source code
Average speed to fast copy UnrealEngine source code on the same drive.<br>
Total size 27gb+ and 161,469 Files.<br>
Speed in MB/s: 290.021765<br>

### BattleField 1 Game files
Average speed to fast copy Battlefield 1 game files on the same drive.<br> 
Total size 81.4 GB and 942 Files.<br>
Speed in MB/s: 731.275685<br>

### Speed may vary
The speed will change depending on OS caching, if you copy the same directory twice it will be faster the second time. If copying from say drive x: to drive y: speed may be faster depending on the ssd and system. I get speeds up to 2GB+/s when copying from my D: drive to my C: drive. But when copying on the same drive average speed maxes out around 700MB/s.