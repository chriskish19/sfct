#include "w32benchmark.hpp"

void application::benchmark::start_clock() noexcept
{
    m_start = std::chrono::high_resolution_clock::now();
}

void application::benchmark::end_clock() noexcept
{
    m_end = std::chrono::high_resolution_clock::now();
}

double_t application::benchmark::speed(std::uintmax_t totalSize) noexcept
{
    if(totalSize == 0ull){
        return 0.0;
    } 
        
    m_duration = m_end - m_start;
    double_t seconds = m_duration.count();
    double_t speed = totalSize / seconds; // Bytes per second
    return speed / 1024 / 1024; // MB/s
}

void application::benchmark::speed_test(const copyto& dir,std::uintmax_t bytes) noexcept
{
    std::string filename = "benchmark_file.dat";  // Name of the file to be created
    
    try{
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
    }
    catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem related errors
        std::cerr << "Filesystem error: " << e.what() << "\n";

        sfct_api::remove_entry(dir.source/filename);
        sfct_api::remove_entry(dir.destination/filename);

        return;
    }
    catch(const std::runtime_error& e){
        // the error message
        std::cerr << "Runtime error: " << e.what() << "\n";

        sfct_api::remove_entry(dir.source/filename);
        sfct_api::remove_entry(dir.destination/filename);

        return;
    }
    catch(const std::bad_alloc& e){
        // the error message
        std::cerr << "Allocation error: " << e.what() << "\n";

        sfct_api::remove_entry(dir.source/filename);
        sfct_api::remove_entry(dir.destination/filename);

        return;
    }
    catch (const std::exception& e) {
        // Catch other standard exceptions
        std::cerr << "Standard exception: " << e.what() << "\n";

        sfct_api::remove_entry(dir.source/filename);
        sfct_api::remove_entry(dir.destination/filename);

        return;
    } catch (...) {
        // Catch any other exceptions
        std::cerr << "Unknown exception caught \n";

        sfct_api::remove_entry(dir.source/filename);
        sfct_api::remove_entry(dir.destination/filename);

        return;
    }


    // start the clock
    benchmark test;
    test.start_clock();
    
    sfct_api::copy_file(dir.source/filename,dir.destination/filename,dir.co);

    // stop the timer
    test.end_clock();

    // get the bench file size
    auto bench_file_size = sfct_api::get_entry_size(dir.source/filename);

    if(bench_file_size.has_value()){
        // speed in MB/s
        double_t speed = test.speed(bench_file_size.value());

        STDOUT << App_MESSAGE("Speed in MB/s: ") << TOSTRING(speed) << "\n";
    }
    else{
        STDOUT << App_MESSAGE("Failed to get the transfer rate") << "\n";
    }

    sfct_api::remove_entry(dir.source/filename);
    sfct_api::remove_entry(dir.destination/filename);
}

void application::benchmark::speed_test_4k(const copyto &dir, std::uintmax_t filesCount, std::uintmax_t bytes) noexcept
{
    std::uintmax_t bytes_per_file = bytes / filesCount;
    
    // keep track of all the files created
    std::vector<STRING> filenames;

    try{
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
    }
    catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem related errors
        std::cerr << "Filesystem error: " << e.what() << "\n";

        // slower but safer than using remove_all
        for(const auto& filename:filenames){
            // clean up files
            sfct_api::remove_entry(dir.source/filename);
            sfct_api::remove_entry(dir.destination/filename);
        }

        return;
    }
    catch(const std::runtime_error& e){
        // the error message
        std::cerr << "Runtime error: " << e.what() << "\n";

        // slower but safer than using remove_all
        for(const auto& filename:filenames){
            // clean up files
            sfct_api::remove_entry(dir.source/filename);
            sfct_api::remove_entry(dir.destination/filename);
        }

        return;
    }
    catch(const std::bad_alloc& e){
        // the error message
        std::cerr << "Allocation error: " << e.what() << "\n";

        // slower but safer than using remove_all
        for(const auto& filename:filenames){
            // clean up files
            sfct_api::remove_entry(dir.source/filename);
            sfct_api::remove_entry(dir.destination/filename);
        }

        return;
    }
    catch (const std::exception& e) {
        // Catch other standard exceptions
        std::cerr << "Standard exception: " << e.what() << "\n";

        // slower but safer than using remove_all
        for(const auto& filename:filenames){
            // clean up files
            sfct_api::remove_entry(dir.source/filename);
            sfct_api::remove_entry(dir.destination/filename);
        }

        return;

    } catch (...) {
        // Catch any other exceptions
        std::cerr << "Unknown exception caught \n";

        // slower but safer than using remove_all
        for(const auto& filename:filenames){
            // clean up files
            sfct_api::remove_entry(dir.source/filename);
            sfct_api::remove_entry(dir.destination/filename);
        }

        return;
    }

    // start the clock
    benchmark test;
    test.start_clock();

    sfct_api::copy_entry(dir.source,dir.destination,dir.co);
    
    // stop the timer
    test.end_clock();

    // speed in MB/s
    double_t speed = test.speed(bytes);

    STDOUT << App_MESSAGE("Speed in MB/s: ") << TOSTRING(speed) << "\n";

    // slower but safer than using remove_all
    for(const auto& filename:filenames){
        // clean up files
        sfct_api::remove_entry(dir.source/filename);
        sfct_api::remove_entry(dir.destination/filename);
    }
    
}

void application::benchmark::speed_test_directories(const std::vector<copyto> &dirs) noexcept
{
    for(const auto& dir: dirs){
        if((dir.commands & cs::four_k) != cs::none){
            // edit values in constants.hpp
            speed_test_4k(dir,FourKFileNumber,FourKTestSize);
        }
        else{
            // 1GB test
            speed_test(dir,TestSize);
        }
    }
}
