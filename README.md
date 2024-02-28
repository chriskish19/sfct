# Simple Folder Copy Tool
Directory monitoring, file syncing, fast file copy, and benchmarking. Using a script to specify commands and arguments for each source and destination directory.

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
## Releases
Download the latest binary release from the releases page.

## Setup
Run sfct.exe for the first time and it will create an sfct_list.txt file in the current directory. This is where you specify the directories you want synced, copied or benchmarked.

sftc_list.txt Example:
```
copy -recursive -update
{
    src C:\test example;
    dst D:\test example;
}

fast_copy -recursive -update
{
    src C:\test;
    dst D:\test;
}

benchmark -create -fast
{
    src C:\benchtest\src;
    dst C:\benchtest\dst;
}

monitor -recursive -update -sync
{
    src C:\test example;
    dst D:\test example;
}
```

There is no limit to the number of directories. Each monitored directory will use 10MB of memory for monitoring so watch that if your adding 100 directories to monitor it will use 1GB of memory.

Add your directories to the sfct_list.txt file, save it and re-run sfct.exe you should see a message in the console that says "Succesfully opened sfct_list.txt".

## Commands and Args
### copy
Checks that files are available and then copies the files. Each file entry that is processed is displayed in the console window.

### fast_copy
Does not check if the files are available. Simply attempts to copy the files.

### monitor
Monitors a directory for changes, when changes occur the program wakes up and performs the arguments specified. Typically recursive, update, and sync. Any changes to dst will not affect src. Changes are not relfected in dst directory immediately their is a delay before actual processing takes place. Each file entry that is processed is displayed in the console window.

### benchmark
Performs a speed test of the copy operation, currently uses std::filesystem::copy under the hood to copy the files. When -4k arg is supplied a large number of small files are created and copied. If -create arg is supplied the directories will be created.

### src
Specify the source directory after this keyword followed by a semi-colon to signify the end of the line.

### dst 
Specify the destination directory after this keyword followed by a semi-colon to signify the end of the line.

### -recursive
Sub-directories are included.

### -update
The existing file is checked and updated to the src version if it is newer.

### -overwrite
The existing file is replaced.

### -create
Creates src and dst directories.

### -sync
Syncs a src directory to a dst directory. When a file or directory is added to src it is added to dst and when a directory or file is removed from src, it is removed from dst. It is a one-way sync.

### -sync_add
Syncs a src directory to a dst directory. When a file or directory is added to src it is also added to dst but when a file or directory is removed from src it is not removed from dst.

### -single
Sub-directories not included only the files in the directory.

### -4k
Performs a benchmark to test copy speed using many small files.

### -fast
This argument is currently not used. It will be implemented in the future.

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
1. No GUI.
2. Windows Only.
3. Needs a terminal window to stay running.
4. No checks to avoid sub-directory recursive monitoring. Currently depends on the user not to double monitor.
 

## Future Plans
1. Linux support.
2. Mac support.
3. Add GUI.
4. Address current limitations.
5. Add more features.
6. Packaged installer.

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

