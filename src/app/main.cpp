#include "ConsoleApp.hpp"
#include <exception>
#include <glew.h>
#include <glfw3.h>

// Main entry point for the application
int main(){
	GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

	// init glew
	if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

	
	std::jthread sfct_t([window](){
		try{
			std::unique_ptr<application::ConsoleApp> sfct = std::make_unique<application::ConsoleApp>();
			sfct->Go();
		}
		catch (const std::filesystem::filesystem_error& e) {
			// Handle filesystem related errors
			std::cerr << "Filesystem error: " << e.what() << "\n";
		}
		catch(const std::runtime_error& e){
			// the error message
			std::cerr << "Runtime error: " << e.what() << "\n";
		}
		catch(const std::bad_alloc& e){
			// the error message
			std::cerr << "Allocation error: " << e.what() << "\n";
		}
		catch (const std::exception& e) {
			// Catch other standard exceptions
			std::cerr << "Standard exception: " << e.what() << "\n";
		} catch (...) {
			// Catch any other exceptions
			std::cerr << "Unknown exception caught \n";
		}
	});
    
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}


	if(sfct_t.joinable()){
		sfct_t.join();
	}
	

	glfwSetWindowShouldClose(window, GL_TRUE);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}