#pragma once

#include <stdio.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"

class Window {
public:
	Window();
	Window(GLint w, GLint h);
	~Window() { glfwTerminate(); };
	int initialise(const char* windowName);
	bool shouldClose() { return glfwWindowShouldClose(mainWindow); };
	void swapBuffers() { glfwSwapBuffers(mainWindow); };
	GLint getBufferWidth() const { return bufferWidth; };
	GLint getBufferHeight() const { return bufferHeight; };
	const bool* getKeys() const { return keys; };
	GLfloat getXChange() { GLfloat c = xChange; xChange = 0.0f; return c; };
	GLfloat getYChange() { GLfloat c = yChange; yChange = 0.0f; return c; };
private:
	GLFWwindow* mainWindow{ nullptr };
	GLint bufferWidth{ 0 }, bufferHeight{ 0 };
	GLint width, height;
	bool keys[1024];
	GLfloat lastX{ 0.0f }, lastY{ 0.0f };
	GLfloat xChange{ 0.0f }, yChange{ 0.0f };
	bool mouseFirstMove{ true };

	static void handleKeys(GLFWwindow* window, int key, int code, int action, int mode);
	static void handleMouse(GLFWwindow* window, double xPos, double yPos);
	void createCallbacks();
};