#pragma once
#include <filesystem>


namespace application{
    class DirectorySignal{
    public:
        DirectorySignal(std::filesystem::path path_to_watch);
    };
}