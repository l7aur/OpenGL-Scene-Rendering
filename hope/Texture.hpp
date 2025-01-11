#pragma once

#include "stb_image.h"
#include <GL/glew.h>

class Texture {
public:
	Texture();
	Texture(const char* fileLocation);
	void loadTexture();
	void useTexture();
	void clearTexture();
	~Texture() { clearTexture(); };
private:
	GLuint textureID{ 0 };
	int width{ 0 }, height{ 0 }, bitDepth{ 0 };
	const char* path{ "" };
};