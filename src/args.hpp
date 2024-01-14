#pragma once
#include <unordered_map>
#include <string>
#include <optional>

namespace application{
    enum class cherry_script{
        copy,
        monitor,
        fast_copy,
        recursive,
        update,
        overwrite,
        single,
        sync,
        sync_add,
        src,
        dst,
        open_brace,
        close_brace,
        semi_colon
    };
    using cs = cherry_script;

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
