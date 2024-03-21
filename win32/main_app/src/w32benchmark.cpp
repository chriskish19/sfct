#include "w32benchmark.hpp"

void application::benchmark::reset_clock() noexcept
{
    // I think this will only throw if system memory is exhausted
    try{
        // attempting to call blank constructors to reset timer
        m_start = std::chrono::steady_clock::time_point();
        m_end = std::chrono::steady_clock::time_point();
        m_duration = std::chrono::duration<double_t>();
    }
    catch(const std::runtime_error& e){
        std::cerr << "Runtime error: " << e.what() << "\n";
    }
    catch(const std::bad_alloc& e){
        std::cerr << "Allocation error: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << "\n";
    } 
    catch (...) {
        std::cerr << "Unknown exception caught \n";
    }
}

void application::benchmark::start_clock() noexcept
{
    try{
        // the operator= can technically throw
        // not much we can do except inform the user that an exception
        // occured, the speed calculation will return 0.0 when called
        m_start = std::chrono::high_resolution_clock::now();
    }
    catch(const std::runtime_error& e){
        std::cerr << "Runtime error: " << e.what() << "\n";
    }
    catch(const std::bad_alloc& e){
        std::cerr << "Allocation error: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << "\n";
    } 
    catch (...) {
        std::cerr << "Unknown exception caught \n";
    }
}

void application::benchmark::end_clock() noexcept
{
    try{
        // the operator= can technically throw
        // not much we can do except inform the user that an exception
        // occured, the speed calculation will return 0.0 when called
        m_end = std::chrono::high_resolution_clock::now();
    }
    catch(const std::runtime_error& e){
        std::cerr << "Runtime error: " << e.what() << "\n";
    }
    catch(const std::bad_alloc& e){
        std::cerr << "Allocation error: " << e.what() << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << "\n";
    } 
    catch (...) {
        std::cerr << "Unknown exception caught \n";
    }
}

double_t application::benchmark::speed(std::uintmax_t totalSize) noexcept
{
    // prevent division by zero
    if(totalSize == 0ull){
        return 0.0;
    } 
    
    try{
        // the operator= can technically throw
        // calculate the time difference
        // if this throws any exception we return 0.0 to avoid 
        // division by zero when speed is calculated 
        m_duration = m_end - m_start;
    }
    catch(const std::runtime_error& e){
        std::cerr << "Runtime error: " << e.what() << "\n";
        return 0.0;
    }
    catch(const std::bad_alloc& e){
        std::cerr << "Allocation error: " << e.what() << "\n";
        return 0.0;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << "\n";
        return 0.0;
    } 
    catch (...) {
        std::cerr << "Unknown exception caught \n";
        return 0.0;
    }

    // get the time dfference in seconds
    double_t seconds = m_duration.count();

    // calculate the rate in bytes per second
    double_t speed = totalSize / seconds;
    
    // return in MB/s
    return speed / 1024 / 1024;
}

void application::benchmark::speed_test(const copyto& dir,std::uintmax_t bytes) noexcept
{
    std::string filename = "benchmark_file.dat";  // Name of the file to be created
    
    try{
        // create a file stream
        std::fstream bench_file;

        // attempt to create the file and open it for writing
        bench_file.open(dir.source/filename, std::ios::out | std::ios::binary); 

        // check that its open before attempting to write data to it
        if(bench_file.is_open()){
            // use this variable to write 100 MB chunks to bench_file
            std::uintmax_t bytes_buffer = 1024ull * 1024 * 100;
            
            // keeps track of total bytes written to the file
            std::uintmax_t total_written{};

            // write 100MB at a time to avoid high ram usage
            while(total_written < bytes){
                // use a vector of chars as the data container allocating 100 MB 
                std::vector<char> data(bytes_buffer);

                // fill the char vector data with the character '0'
                std::fill(data.begin(), data.end(), '0');

                // write the all the '0' chars into bench_file
                bench_file.write(data.data(), data.size());

                // accumulate the bytes written and store it in total_written
                total_written += bytes_buffer;
            }

            // we have completed writing data to bench_file so now we can close it
            bench_file.close();
        }
        else{
            // bench_file failed to open
            // return the function back to the caller bypassing
            // the rest of the code
            return;
        }
        
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";

        // attempt to remove the created file
        // if that fails theres not much we can do
        sfct_api::remove_entry(dir.source/filename);
        sfct_api::remove_entry(dir.destination/filename);

        // bench_file automatically closes when out of scope
        // return to the caller bypassing the rest of the code
        // avoiding the speed test
        return;
    }
    catch(const std::runtime_error& e){
        std::cerr << "Runtime error: " << e.what() << "\n";

        sfct_api::remove_entry(dir.source/filename);
        sfct_api::remove_entry(dir.destination/filename);

        return;
    }
    catch(const std::bad_alloc& e){
        std::cerr << "Allocation error: " << e.what() << "\n";

        sfct_api::remove_entry(dir.source/filename);
        sfct_api::remove_entry(dir.destination/filename);

        return;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << "\n";

        sfct_api::remove_entry(dir.source/filename);
        sfct_api::remove_entry(dir.destination/filename);

        return;
    } 
    catch (...) {
        std::cerr << "Unknown exception caught \n";

        sfct_api::remove_entry(dir.source/filename);
        sfct_api::remove_entry(dir.destination/filename);

        return;
    }

    // create a benchmark object to calculate the transfer rate
    // of the copy operation
    benchmark test;

    // start the timer
    test.start_clock();
    
    // attempt to copy the file
    // if this fails it will log the error to the console
    sfct_api::copy_file(dir.source/filename,dir.destination/filename,dir.co);

    // stop the timer
    test.end_clock();

    // get the bench file size
    auto bench_file_size = sfct_api::get_entry_size(dir.source/filename);

    // check that bench_file_size was successful(has a value)
    if(bench_file_size.has_value()){
        double_t speed = test.speed(bench_file_size.value());                   // calculate the speed in MB/s
        STDOUT << App_MESSAGE("Speed in MB/s: ") << TOSTRING(speed) << "\n";    // inform the user of the rate
    }
    else{
        STDOUT << App_MESSAGE("Failed to get the transfer rate") << "\n";       // inform the user that the operation failed
    }

    sfct_api::remove_entry(dir.source/filename);                                // remove the file from source directory
    sfct_api::remove_entry(dir.destination/filename);                           // remove the file from destination directory
}

void application::benchmark::speed_test_4k(const copyto &dir, std::uintmax_t filesCount, std::uintmax_t bytes) noexcept
{
    // prevent division by zero and no files being created
    // so exit the function
    if(filesCount == 0){
        return;
    }
    
    // calculate the size in bytes of each file
    std::uintmax_t bytes_per_file = bytes / filesCount;
    
    // keeps track of all the files created so we can clean them up
    std::vector<STRING> filenames;

    // if the code in the try block throws an exception(any exception) we delete any created files
    // and return the function back to the caller
    try{ 
        // attempt to create filesCount number of files for speed testing
        for(std::uintmax_t i{};i<filesCount;i++){
            // create a unique file name for each file
            STRING filename = App_MESSAGE("benchmark_file") + TOSTRING(i) + App_MESSAGE(".dat");
            
            // add the file name to the vector to keep track of them
            filenames.push_back(filename);

            // create the file stream object
            std::fstream bench_file;

            // attempt to create the file and open it
            bench_file.open(dir.source/filename,std::ios::out | std::ios::binary);
            
            // check that it is actually open before attempting to write to it
            // if it isnt open let the for loop move on and try the next file
            if(bench_file.is_open()){
                std::uintmax_t total_written{};                     // keeps track of total data written to bench_file
                std::uintmax_t bytes_buffer{1024ull*1024*100};      // used to keep ram usage at 100 MB


                // if bytes_per_file is greater than bytes_buffer then we need to break up data writes
                // into chunks to limit ram usage else we can do it in a single write operation.
                if(bytes_per_file > bytes_buffer){
                    
                    // get the remainder of dividing bytes_per_file by bytes_buffer
                    // this gives an amount less than bytes_buffer.
                    std::uintmax_t left_over{bytes_per_file % bytes_buffer};

                    // write the data to bench file in chunks(100 MB) until it equals
                    // bytes_per_file, write the first chunk size as variable to get exactly
                    // bytes_per_file. The first iteration will write a chunk size less than
                    // bytes_buffer this is because it is a remainder of dividing bytes_per_file
                    // by bytes_buffer.
                    while(total_written < bytes_per_file){
                        // amount of data to write on each iteration
                        // if left_over is not zero then allocate_buffer will be set to 
                        // left_over's value. If left_over is zero then allocate_buffer will be
                        // set to bytes_buffer's value.
                        std::uintmax_t allocate_buffer{left_over ? left_over : bytes_buffer };
                        
                        // allocate allocate_buffer using a vector of chars called data 
                        std::vector<char> data(allocate_buffer);

                        // fill the vector data with the character '0'
                        std::fill(data.begin(), data.end(), '0');

                        // write the vector data to bench_file
                        bench_file.write(data.data(), data.size());

                        // accumulate the amount of data written
                        total_written += allocate_buffer;

                        // set left_over for the next iteration
                        // it should equal zero
                        left_over = 0;
                    }

                    // we have finished so close the file
                    bench_file.close();
                }
                else{
                    // allocate bytes_per_file using a vector of chars called data 
                    std::vector<char> data(bytes_per_file);

                    // fill the vector data with the character '0'
                    std::fill(data.begin(), data.end(), '0');

                    // write the vector data to bench_file
                    bench_file.write(data.data(), data.size());

                    // we have finished so close the file
                    bench_file.close();
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";

        // clean up any files that were created
        for(const auto& filename:filenames){
            sfct_api::remove_entry(dir.source/filename);
            sfct_api::remove_entry(dir.destination/filename);
        }
        
        // exit
        return;
    }
    catch(const std::runtime_error& e){
        std::cerr << "Runtime error: " << e.what() << "\n";

        for(const auto& filename:filenames){
            sfct_api::remove_entry(dir.source/filename);
            sfct_api::remove_entry(dir.destination/filename);
        }

        return;
    }
    catch(const std::bad_alloc& e){
        std::cerr << "Allocation error: " << e.what() << "\n";

        for(const auto& filename:filenames){
            sfct_api::remove_entry(dir.source/filename);
            sfct_api::remove_entry(dir.destination/filename);
        }

        return;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << "\n";

        for(const auto& filename:filenames){
            sfct_api::remove_entry(dir.source/filename);
            sfct_api::remove_entry(dir.destination/filename);
        }

        return;

    } 
    catch (...) {
        std::cerr << "Unknown exception caught \n";

        for(const auto& filename:filenames){
            sfct_api::remove_entry(dir.source/filename);
            sfct_api::remove_entry(dir.destination/filename);
        }

        return;
    }

    // create a benchmark object for speed testing
    benchmark test;

    // start the timer
    test.start_clock();

    // this will copy the entire directory source to destination
    // it could be a problem if the directory contains file entries in it
    // that are not part of the test
    sfct_api::copy_entry(dir.source,dir.destination,dir.co);
    
    // stop the timer
    test.end_clock();

    // get the speed in MB/s
    double_t speed = test.speed(bytes);

    // inform the user of transfer speed
    STDOUT << App_MESSAGE("Speed in MB/s: ") << TOSTRING(speed) << "\n";

    // clean up the created files
    for(const auto& filename:filenames){
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
