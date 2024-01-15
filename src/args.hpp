#pragma once
#include <unordered_map>
#include <string>
#include <optional>

/////////////////////////////////////////////////////////////////////
// This header contains everything needed for tokenizing the commands from strings into enum values.
// cherry_script is defined here.
// global_tokenizer is defiend here.
/////////////////////////////////////////////////////////////////////


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
    };
    using cs = cherry_script;

    inline cs operator|(cs a, cs b) {
        return static_cast<cs>(static_cast<int>(a) | static_cast<int>(b));
    }

    inline cs operator&(cs a, cs b) {
        return static_cast<cs>(static_cast<int>(a) & static_cast<int>(b));
    }
    inline cs& operator|=(cs& a, cs b) {
        return a = a | b;
    }

    class args_maps{
    public:
        std::optional<cs> Find(const std::string& arg){
            auto command_arg = m_command_mp.find(arg);
            if(command_arg != m_command_mp.end()){
                return std::optional<cs>(command_arg->second);
            }
            return std::nullopt;
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
                                                            {";",cs::semi_colon}   };
    };

    inline args_maps global_tokenizer;
}
