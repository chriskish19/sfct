#include "FileCopy.hpp"

void application::FileCopy::copy_file(){
    


}

application::FileCopy::FileCopy(std::filesystem::copy_options co,std::shared_ptr<std::vector<copyto>> directories)
:m_CO(co),m_Directories(directories){

}