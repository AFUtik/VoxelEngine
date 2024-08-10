#ifndef WINDOW_WINDOW_HPP_
#define WINDOW_WINDOW_HPP_

struct GLFWwindow;

class Window {
public:
	static GLFWwindow* window;
	static int init(int width, int height, const char* title);
	static void terminate();

	static bool isShouldClose();
	static void setShouldClose(bool flag);
	static void swapBuffers();
};

#endif
