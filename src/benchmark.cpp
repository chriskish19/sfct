#include "benchmark.hpp"

void application::benchmark::start_clock()
{
    m_start = std::chrono::high_resolution_clock::now();
}

void application::benchmark::end_clock()
{
    m_end = std::chrono::high_resolution_clock::now();
}

double application::benchmark::speed(std::uintmax_t totalSize)
{
    m_duration = m_end - m_start;
    double seconds = m_duration.count();
    double speed = totalSize / seconds; // Bytes per second
    return speed / 1024 / 1024; // MB/s
}

void application::benchmark::speed_test(const copyto& dir,std::uintmax_t bytes)
{
    std::string filename = "benchmark_file.dat";  // Name of the file to be created
    std::fstream bench_file;

    bench_file.open(dir.source/filename, std::ios::out | std::ios::binary); 

    // write 100MB at a time
    std::uintmax_t bytes_buffer = 1024ull * 1024 * 100;
    std::uintmax_t total_written{};
    while(total_written < bytes){
        std::vector<char> data(bytes_buffer);
        std::fill(data.begin(), data.end(), '0');
        bench_file.write(data.data(), data.size());
        total_written += bytes_buffer;
    }
    bench_file.close();

    // start the clock
    benchmark test;
    test.start_clock();

    if((dir.commands & cs::fast) != cs::none){
        std::filesystem::path srcfilepath(dir.source/filename);
        std::filesystem::path dstfilepath(dir.destination/filename);
        Windows::FastCopy(srcfilepath.c_str(),dstfilepath.c_str());
    }
    else{
        std::filesystem::copy(dir.source,dir.destination,dir.co);
    }
    

    // stop the timer
    test.end_clock();

    // get the bench file size
    std::uintmax_t bench_file_size = std::filesystem::file_size(dir.source/filename);

    // speed in MB/s
    double speed = test.speed(bench_file_size);

    m_MessageStream.SetMessage(App_MESSAGE("Speed in MB/s: ") + TOSTRING(speed));
    m_MessageStream.ReleaseBuffer();

    if(std::filesystem::exists(dir.source/filename)){
        // clean up file
        if(!std::filesystem::remove(dir.source/filename)){
            logger log(App_MESSAGE("Failed to remove file"),Error::DEBUG,dir.source/filename);
            log.to_console();
            log.to_log_file();
        }
    }
    
    if(std::filesystem::exists(dir.destination/filename)){
        if(!std::filesystem::remove(dir.destination/filename)){
            logger log(App_MESSAGE("Failed to remove file"),Error::DEBUG,dir.destination/filename);
            log.to_console();
            log.to_log_file();
        }
    }
    
}

void application::benchmark::speed_test_4k(const copyto &dir, std::uintmax_t filesCount, std::uintmax_t bytes)
{
    std::uintmax_t bytes_per_file = bytes / filesCount;
    
    // keep track of all the files created
    std::vector<STRING> filenames;

    // create many small files
    for(std::uintmax_t i{};i<filesCount;i++){
        STRING filename = App_MESSAGE("benchmark_file") + TOSTRING(i) + App_MESSAGE(".dat");
        filenames.push_back(filename);
        std::fstream bench_file;
        bench_file.open(dir.source/filename,std::ios::out | std::ios::binary);
        std::vector<char> data(bytes_per_file);
        std::fill(data.begin(), data.end(), '0');
        bench_file.write(data.data(), data.size());
        bench_file.close();
    }

    // start the clock
    benchmark test;
    test.start_clock();

    if((dir.commands & cs::fast) != cs::none){
        Windows::MTFastCopy(dir);
    }
    else{
        std::filesystem::copy(dir.source,dir.destination,dir.co);
    }

    // stop the timer
    test.end_clock();

    // speed in MB/s
    double speed = test.speed(bytes);

    m_MessageStream.SetMessage(App_MESSAGE("Speed in MB/s: ") + TOSTRING(speed));
    m_MessageStream.ReleaseBuffer();

    // slower but safer than using remove_all
    for(const auto& filename:filenames){
        // clean up files
        if(std::filesystem::exists(dir.source/filename)){
            if(!std::filesystem::remove(dir.source/filename)){
                logger log(App_MESSAGE("Failed to remove file: "),Error::DEBUG,dir.source/filename);
                log.to_console();
                log.to_log_file();
            }
        }
        
        if(std::filesystem::exists(dir.destination/filename)){
            if(!std::filesystem::remove(dir.destination/filename)){
                logger log(App_MESSAGE("Failed to remove file: "),Error::DEBUG,dir.destination/filename);
                log.to_console();
                log.to_log_file();
            }
        }
    }
    
}
