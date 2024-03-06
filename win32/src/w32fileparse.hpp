#pragma once
#include "w32cpplib.hpp"
#include "w32logger.hpp"
#include "w32args.hpp"
#include "w32obj.hpp"
#include "w32sfct_api.hpp"


//////////////////////////////////////////////////////////////////
// This header is responsible for parsing a file.               //
// It extracts the key information.                             //
// In this case it extracts directory paths, commands.          //
//////////////////////////////////////////////////////////////////


namespace application{
    // to use this class first call one of the constructors with a full path or a filename relative to 
    // the cwd(current working directory) then call functions in this order:
    // 1. OpenFile()
    // 2. ExtractData()
    // 3. CheckData() 
    class FileParse{
    public:
        /// @brief default destructor
        ~FileParse()= default;
        
        /// @brief Copy constructor
        FileParse(const FileParse&) = delete;

        /// @brief Copy assignment operator
        FileParse& operator=(const FileParse&) = delete;

        /// @brief Move constructor
        FileParse(FileParse&&) = delete;

        /// @brief Move assignment operator
        FileParse& operator=(FileParse&&) = delete;

        /// @brief use this constructor if you need to use a file not in the current working directory
        /// must be an absolute path
        /// @param path the file name including extension and full path
        FileParse(const std::filesystem::path& path) noexcept;

        /// @brief use this constructor if the file being parsed is in the current working directory
        /// @param filename the name of the file to be parsed
        FileParse(const std::string& filename) noexcept;

        /// @brief parse the file into m_Data
        void ExtractData() noexcept;
        
        /// @brief opens m_File
        /// if the path is valid but opening the file fails std::runtime_error exception is thrown
        /// if the path is invalid the function returns false
        bool OpenFile() noexcept;
        
        /// @brief changes m_FilePath to a new path and resets the class members
        /// then its safe to call OpenFile agian and ExtractData with the same object
        void SetFilePath(const std::filesystem::path& new_path) noexcept;

        /// @brief checks for valid data
        void CheckData() noexcept;

        /// @brief data will need to be used elsewhere in the program
        const std::shared_ptr<std::vector<copyto>> GetSPdata() noexcept {return m_Data;}

    private:
        /// @brief extracts the data from the text file
        void ParseSyntax();

        /// @brief the path to the current file being parsed
        std::filesystem::path m_FilePath;

        /// @brief file to parse
        std::ifstream m_File;

        /// @brief holds the parsed data from the file
        std::shared_ptr<std::vector<copyto>> m_Data{std::make_shared<std::vector<copyto>>()};

        /// @brief flag for whether the file exists or not
        bool m_FileExists{false};

        /// @brief flag for whether the data has been extracted or not
        bool m_DataExtracted{false};

        /// @brief checks m_Data for valid directory entries
        /// if no valid directories are found it throws an exception std::runtime_error()
        void CheckDirectories() noexcept;

        /// @brief checks for valid commands
        /// @param commands enum of copy arguments
        /// @return false for invalid commands, true for valid commands
        bool ValidCommands(cs commands) noexcept;

        /// @brief gets the copy arguments from the text file and puts them into a cs enum object
        /// @param lineStream a single line of text input
        /// @return a cs enum object with the copy arguments
        cs ParseCopyArgs(std::istringstream& lineStream);
        
        /// @brief extracts the source and destination paths from the text file and puts them into a copyto reference
        /// @param dir a copyto reference object
        void ParseDirs(copyto& dir);

        /// @brief extracts the monitor arguments
        /// @param lineStream a single line of text input
        /// @return a cs enum which holds the monitor arguments
        cs ParseMonitorArgs(std::istringstream& lineStream);

        /// @brief extracts the benchmark arguments
        /// @param lineStream a single line of text input
        /// @return a cs enum which holds the benchmark arguments
        cs ParseBenchArgs(std::istringstream& lineStream);

        /// @brief text to cs enum converter
        args_maps tokenizer;
    };
}
