#include "ConsoleApp.hpp"
#include <stdexcept>

// Main entry point for the application
int main(){
    try{
		using namespace application;
		std::unique_ptr<ConsoleApp> sfct{std::make_unique<ConsoleApp>()};
		sfct->Go();
	}
	catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem related errors
        std::cerr << "Filesystem error: " << e.what() << '\n';
	}
	catch(const std::runtime_error& e){
		// the error message
		std::cerr << e.what() << std::endl;
		
		// exit the program there is no recovering
		return 1;
	}
	catch(const std::bad_alloc& e){
		// the error message
		std::cerr << e.what() << std::endl;

		// exit the program there is no recovering
		return 2;
	}
	catch (const std::exception& e) {
        // Catch other standard exceptions
        std::cerr << "Standard exception: " << e.what() << '\n';
    } catch (...) {
        // Catch any other exceptions
        std::cerr << "Unknown exception caught\n";
    }
	return 0;
}