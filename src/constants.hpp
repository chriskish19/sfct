#pragma once
#include <cstdint>
// max file size for Windows::FastCopy
inline constexpr std::uintmax_t MaxFileSize = 1024ull * 1024 * 1024; // 1GB

// min file size for Windows::FastCopy
inline constexpr std::uintmax_t MinFileSize = 1024ull * 1024; // 1MB

// test size for the benchmark standard 4k test
inline constexpr std::uintmax_t FourKTestSize = 1024ull * 1024 * 1024; // 1GB

// number of files to use for the standard 4k test
inline constexpr std::uintmax_t FourKFileNumber = 10000; // 10,000

// standard test size used in speed test
inline constexpr std::uintmax_t TestSize = 1024ull * 1024 * 1024; // 1GB

// monitor buffer size
inline constexpr std::uintmax_t MonitorBuffer = 1024ull * 1024 * 10; // 10MB