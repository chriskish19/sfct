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

void application::benchmark::speed_test(std::uintmax_t bytes)
{
    std::string filename = "benchmark_file.dat";  // Name of the file to be created
    std::fstream bench_file;

    std::filesystem::path src(exe_path / filename);
    std::filesystem::path dst(exe_path/"benchtest");

    bench_file.open(src, std::ios::out | std::ios::binary); 

    std::vector<char> data(bytes);
    std::fill(data.begin(), data.end(), '0');

    bench_file.write(data.data(), data.size());
    bench_file.close();

    // Now create a folder to copy it to
    if(!std::filesystem::exists(dst)){
         if(!std::filesystem::create_directory(dst)){
            logger log(App_MESSAGE("Failed to create benchmark directory"), Error::DEBUG);
            log.to_console();
            log.to_log_file();
        }
    }
   

    // start the clock
    benchmark test;
    test.start_clock();

    // now try to copy the file
    std::filesystem::copy(src,dst,std::filesystem::copy_options::overwrite_existing);

    // stop the timer
    test.end_clock();

    // get the bench file size
    std::uintmax_t bench_file_size = std::filesystem::file_size(src);

    // speed in MB/s
    double speed = test.speed(bench_file_size);

    m_MessageStream.SetMessage(App_MESSAGE("Speed in MB/s: ") + TOSTRING(speed));
    m_MessageStream.ReleaseBuffer();
}
