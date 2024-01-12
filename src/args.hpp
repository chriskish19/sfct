#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <optional>
#include <variant>
#include <filesystem>


namespace application{
    enum class command{
        copy,
        monitor,
        fast_copy
    };

    enum class copy{
        recursive,
        update,
        overwrite,
        single
    };

    enum class monitor{
        sync,
        sync_add
    };

    enum class directory_syntax{
        src,
        dst,
        open_brace,
        close_brace,
        semi_colon
    };
    using ds = directory_syntax;


    struct args{
        command cmd_arg;
        std::unordered_set<copy> copy_args;
        monitor monitor_arg;
    };


    using map_type = std::variant<command, copy, monitor,ds>;


    class args_maps{
    public:
        std::optional<map_type> Find(const std::string& arg){
            auto command_arg = m_command_mp.find(arg);
            auto copy_arg = m_copy_mp.find(arg);
            auto monitor_arg = m_monitor_mp.find(arg);
            auto ds_token = m_ds_mp.find(arg);

            if(command_arg != m_command_mp.end()){
                return std::optional<map_type>(command_arg->second);
            }
            else if(copy_arg != m_copy_mp.end()){
                return std::optional<map_type>(copy_arg->second);
            }
            else if(monitor_arg != m_monitor_mp.end()){
                return std::optional<map_type>(monitor_arg->second);
            }
            else if(ds_token != m_ds_mp.end()){
                return std::optional<map_type>(ds_token->second);
            }
            return std::nullopt;
        }

    private:
        std::unordered_map<std::string,command> m_command_mp{ {"copy",command::copy},
                                                            {"monitor",command::monitor},
                                                            {"fast_copy",command::fast_copy} };

        std::unordered_map<std::string,copy> m_copy_mp{   {"-recursive",copy::recursive},
                                                        {"-update",copy::update}, 
                                                        {"-overwrite",copy::overwrite},
                                                        {"-single",copy::single}  };

        std::unordered_map<std::string,monitor> m_monitor_mp{ {"-sync",monitor::sync},
                                                            {"-sync_add",monitor::sync_add} };

        
        std::unordered_map<std::string,directory_syntax> m_ds_mp{ {"src",ds::src},
                                                                  {"dst",ds::dst},
                                                                  {"{",ds::open_brace},
                                                                  {"}",ds::close_brace},
                                                                  {";",ds::semi_colon}  };
    };

    inline args_maps global_tokenizer;
}
