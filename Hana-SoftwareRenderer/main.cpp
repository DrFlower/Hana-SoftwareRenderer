#include "iostream"
#include "platform.h"

static const char* const WINDOW_TITLE = "Viewer";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

int main()
{
	std::cout << "hello world" << std::endl;
	platform_initialize();
	window_t* window;

	window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	//window_destroy(window);
}