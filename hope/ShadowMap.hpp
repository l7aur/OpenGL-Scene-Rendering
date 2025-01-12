#pragma once

#include <stdio.h>
#include "GL/glew.h"

class ShadowMap {
public:
	ShadowMap() {};
	~ShadowMap();
	virtual bool init(unsigned int width, unsigned int height);
	virtual void read(GLenum texUnit);
	virtual void write();
	GLuint getShadowWidth() const { return shadowWidth; };
	GLuint getShadowHeight() const { return shadowHeight; };
	GLuint getTexture() const { return shadowMap; };
protected:
	GLuint FBO{ 0 }, shadowMap{ 0 }; 
	GLuint shadowWidth{ 0 }, shadowHeight{ 0 };
};