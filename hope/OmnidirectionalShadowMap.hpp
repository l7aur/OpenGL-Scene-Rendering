#pragma once

#include "ShadowMap.hpp"

class OmnidirectionalShadowMap : public ShadowMap 
{
public:
	OmnidirectionalShadowMap();
	~OmnidirectionalShadowMap() {};
	bool init(unsigned int width, unsigned int height);
	void read(GLenum textureUnit);
private:

};