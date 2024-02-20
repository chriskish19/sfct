#include "ConsoleApp.hpp"
#include <stdexcept>


// Main entry point for the application
int main(){
    
	try{
		std::unique_ptr<application::ConsoleApp> sfct = std::make_unique<application::ConsoleApp>();
		sfct->Go();
	}
	catch (const std::filesystem::filesystem_error& e) {
		// Handle filesystem related errors
		std::cerr << "Filesystem error: " << e.what() << '\n';

		return 1;
	}
	catch(const std::runtime_error& e){
		// the error message
		std::cerr << e.what() << std::endl;
		
		return 2;
	}
	catch(const std::bad_alloc& e){
		// the error message
		std::cerr << e.what() << std::endl;

		return 3;
	}
	catch (const std::exception& e) {
		// Catch other standard exceptions
		std::cerr << "Standard exception: " << e.what() << '\n';

		return 4;
	} catch (...) {
		// Catch any other exceptions
		std::cerr << "Unknown exception caught\n";

		return 5;
	}
	
	
	return 0;
}