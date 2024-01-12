#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <filesystem>
#include "logger.hpp"
#include <sstream>
#include "args.hpp"


namespace application{
    enum class DataType{
        Directory,
        Text
        // add more for future use of this class in building other cli tools
    };

    struct copyto{
        std::filesystem::path source;
        std::filesystem::path destination;
        args cmd_args;
    };

    // to use this class first call one of the constructors with a full path or a filename relative to 
    // the cwd(current working directory) then call functions in this order:
    // 1. OpenFile()
    // 2. ExtractData()
    // 3. CheckData() 
    class FileParse{
    public:
        struct VariantVisitor {
            void operator()(application::command cmd) const;
            void operator()(application::copy cp) const;
            void operator()(application::monitor mn) const;
            void operator()(application::ds ds_arg) const;
        };

        // use this constructor if you need to use a file not in the current working directory
        // must be an absolute path
        // std::filesystem::path path: the file name including extension and full path
        FileParse(const std::filesystem::path& path);

        // use this constructor if the file being parsed is in the current working directory
        FileParse(const std::string& filename);

        // parse the file into m_Data
        void ExtractData();
        
        // opens m_File
        // if the path is valid but opening the file fails std::runtime_error exception is thrown
        // if the path is invalid the function returns false
        bool OpenFile();
        
        // changes m_FilePath to a new path and resets the class members
        // then its safe to call OpenFile agian and ExtractData with the same object
        void SetFilePath(const std::filesystem::path& new_path);

        // checks for valid data
        void CheckData(DataType t);

        // data will need to be used elsewhere in the program
        // TODO: template this class to handle any type of data for future use in making cli programs
        const std::shared_ptr<std::vector<copyto>> GetSPdata(){return m_Data;}

    private:
        void ParseSyntax();

        // the path to the current file being parsed
        std::filesystem::path m_FilePath;

        // file to parse
        std::ifstream m_File;

        // holds the parsed data from the file
        std::shared_ptr<std::vector<copyto>> m_Data{std::make_shared<std::vector<copyto>>()};

        // flag for whether the file exists or not
        bool m_FileExists{false};

        // flag for whether the data has been extracted or not
        bool m_DataExtracted{false};

        // This function is called from CheckData() if the flag is Directory
        // checks m_Data for valid directory entries
        // if no valid directories are found it throws an exception std::runtime_error()
        void CheckDirectories();

        // This function is called from CheckData() if the flag is Text
        // checks m_Data for valid Text
        void CheckText();
    };
}
