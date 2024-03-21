#include "w32cpplib.hpp"
#include "w32consoleapp.hpp"

int main(){
    std::unique_ptr<application::ConsoleApp> sfct{std::make_unique<application::ConsoleApp>()};
    sfct->Go();
    return 0;
}