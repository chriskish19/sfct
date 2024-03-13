#pragma once
#include "w32cpplib.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////
// This header contains everything needed for tokenizing the commands from strings into enum values. //
// cherry_script is defined here.                                                                    //                                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////////////


namespace application{
    enum class cherry_script {
        none = 0,               // Default value representing no flags set
        copy = 1 << 0,          // 1
        monitor = 1 << 1,       // 2
        fast_copy = 1 << 2,     // 4
        recursive = 1 << 3,     // 8
        update = 1 << 4,        // 16
        overwrite = 1 << 5,     // 32
        single = 1 << 6,        // 64
        sync = 1 << 7,          // 128
        sync_add = 1 << 8,      // 256
        src = 1 << 9,           // 512
        dst = 1 << 10,          // 1024
        open_brace = 1 << 11,   // 2048
        close_brace = 1 << 12,  // 4096
        semi_colon = 1 << 13,   // 8192
        benchmark = 1 << 14,    // 16384
        create = 1 << 15,       // 32768
        four_k = 1 << 16,       // 65536
        fast = 1 << 17          // 131072
    };

    using cs = cherry_script;       // Alias for cherry_script for convenience.

    // Overload bitwise OR operator to combine cherry_script flags.
    inline cs operator|(cs a, cs b) {
        return static_cast<cs>(static_cast<int>(a) | static_cast<int>(b));
    }

    // Overload bitwise AND operator to intersect cherry_script flags.
    inline cs operator&(cs a, cs b) {
        return static_cast<cs>(static_cast<int>(a) & static_cast<int>(b));
    }

    // Overload bitwise OR assignment operator to update cherry_script flags by combining them.
    inline cs& operator|=(cs& a, cs b) {
        return a = a | b;
    }

    // Defines a class to map string arguments to their corresponding cherry_script enum values.
    class args_maps{
    public:
        // Find the enum value corresponding to a string command argument.
        std::optional<cs> Find(const std::string& arg){
            auto command_arg = m_command_mp.find(arg);          // Search for the argument in the map.
            if(command_arg != m_command_mp.end()){              // If found, return the associated enum value.
                return std::optional<cs>(command_arg->second);  
            }
            return std::nullopt;                                // If not found, return an empty optional.
        }

    private:
        std::unordered_map<std::string,cs> m_command_mp{    {"copy",cs::copy},
                                                            {"monitor",cs::monitor},
                                                            {"fast_copy",cs::fast_copy}, 
                                                            {"-recursive",cs::recursive},
                                                            {"-update",cs::update}, 
                                                            {"-overwrite",cs::overwrite},
                                                            {"-single",cs::single},
                                                            {"-sync",cs::sync},
                                                            {"-sync_add",cs::sync_add},
                                                            {"src",cs::src},
                                                            {"dst",cs::dst},
                                                            {"{",cs::open_brace},
                                                            {"}",cs::close_brace},
                                                            {";",cs::semi_colon},
                                                            {"benchmark", cs::benchmark},
                                                            {"-create", cs::create},
                                                            {"-4k",cs::four_k},
                                                            {"fast",cs::fast} };
    };
}
