#include "ConsoleApp.hpp"
#include <stdexcept>

int main(){
    try{
		application::ConsoleApp sfct;
        sfct.Go();
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
	catch (const std::filesystem::filesystem_error& e) {
        // Handle filesystem related errors
        std::cerr << "Filesystem error: " << e.what() << '\n';
        if (e.code() == std::errc::no_such_file_or_directory) {
            std::cerr << "Source or destination does not exist.\n";
        } else if (e.code() == std::errc::file_exists) {
            std::cerr << "Destination file exists and overwrite not specified.\n";
        }
        // Add more specific error handling as needed
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