#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "GLFW/glfw3.h"

class Camera {
public:
	Camera() {};
	Camera(
		glm::vec3 startPosition, glm::vec3 startUp, 
		GLfloat startYaw, GLfloat startPitch,
		GLfloat startMoveSpeed, GLfloat startTurnSpeed);
	void keyControl(const bool* keys, GLfloat dTime);
	void mouseControl(GLfloat xChange, GLfloat yChange);
	glm::mat4 computeViewMatrix();
	glm::vec3 getPosition() const { return position; };
	glm::vec3 getDirection() const { return glm::normalize(front); };
	~Camera() {};
private:
	glm::vec3 position{ 0.0f, 0.0f, 0.0f };
	glm::vec3 front{ 0.0f, 0.0f, -1.0f };
	glm::vec3 up{ 0.0f, 1.0f, 0.0f };
	glm::vec3 right{ 1.0f, 0.0f, 0.0f };
	glm::vec3 worldUp{ 0.0f, 1.0f, 0.0f };

	GLfloat yaw{ 0.0f }, pitch{ 0.0f };
	GLfloat moveSpeed{ 5.0f }, turnSpeed{ 5.0f };

	void update();
};