#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CommonValues.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "Texture.hpp"
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "Material.hpp"
#include "Model.hpp"


Window mainWindow;
Camera camera;
DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];
std::vector<Mesh*> meshList;
std::vector<Shader*> shaderList;
Shader directionalShadowShader;

GLuint uniformProjection{ 0 }, uniformModel{ 0 }, uniformView{ 0 };
GLuint uniformEyePosition{ 0 }, uniformShininess{ 0 }, uniformSpecularI{ 0 };

GLfloat deltaTime{ 0.0f };
GLfloat lastTime{ 0.0f };

unsigned int pointLightCount = 0;
unsigned int spotLightCount = 0;

Material shiny, dull;
Texture brickTex, dirtTex, plainTex;
Model xWing, blackHawk, quad;

static void computeDeltaTime() {
	GLfloat currTime = (GLfloat)glfwGetTime();
	deltaTime = currTime - lastTime;
	lastTime = currTime;
}

static void calcAvgNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount, 
	unsigned int vLength, unsigned int normalOffset) {
	for (size_t i = 0; i < indiceCount; i += 3) {
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);
		in0 += normalOffset;in1 += normalOffset;in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}
	for (size_t i = 0; i < verticeCount / vLength; i++) {
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}

static void createObjects() {
	GLfloat vertices[] = {
		-1.0f, -1.0f, -0.6f,		0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 1.0f,		0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, -0.6f,		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,		0.5f, 1.0f, 0.0f, 0.0f, 0.0f
	};
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f, 10.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f, 0.0f, 10.0f, 0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f, 10.0f, 10.0f, 0.0f, -1.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat quadVertices[] = {
		-5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		-5.0f, 0.0f, -5.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		-5.0f, 5.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		-5.0f, 5.0f, -5.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f
	};
	unsigned int quadIndices[] = {
		0, 1, 2,
		1, 2, 3
	};

	calcAvgNormals(indices, 12, vertices, 32, 8, 5);
	Mesh* obj1 = new Mesh();
	obj1->createMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);
	Mesh* obj2 = new Mesh();
	obj2->createMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);
	Mesh* obj3 = new Mesh();
	obj3->createMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);
	Mesh* quad = new Mesh();
	meshList.push_back(quad);
	quad->createMesh(quadVertices, quadIndices, 32, 6);
}

static void createShaders() {
	Shader* shader = new Shader();
	shader->createFromFiles("shaders/main.vert", "shaders/main.frag");
	shaderList.push_back(shader);

	directionalShadowShader.createFromFiles("shaders/directionalShadow.vert", "shaders/directionalShadow.frag");
}

static void cleanup() {
	for (auto& i : meshList)
		delete i;
	for (auto& i : shaderList)
		delete i;
}

static void renderScene() {
	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.5f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	brickTex.useTexture();
	shiny.useMaterial(uniformSpecularI, uniformShininess);
	meshList.at(0)->renderMesh();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 4.0f, -2.5f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dirtTex.useTexture();
	dull.useMaterial(uniformSpecularI, uniformShininess);
	meshList.at(1)->renderMesh();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dirtTex.useTexture();
	shiny.useMaterial(uniformSpecularI, uniformShininess);
	meshList.at(2)->renderMesh();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-7.0f, 0.0f, 10.0f));
	model = glm::scale(model, glm::vec3(0.006f, 0.006f, 0.006f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	shiny.useMaterial(uniformSpecularI, uniformShininess);
	xWing.renderModel();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-3.0f, 2.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
	model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	shiny.useMaterial(uniformSpecularI, uniformShininess);
	blackHawk.renderModel();

	model = glm::mat4(1.0f);
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dull.useMaterial(uniformSpecularI, uniformShininess);
	meshList.at(3)->renderMesh();
}

void renderPass(glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {
	shaderList.at(0)->useShader();

	uniformModel = shaderList.at(0)->getModelLoc();
	uniformProjection = shaderList.at(0)->getProjectionLoc();
	uniformView = shaderList.at(0)->getViewLoc();
	uniformEyePosition = shaderList.at(0)->getEyePositionLoc();
	uniformSpecularI = shaderList.at(0)->getSpecularILoc();
	uniformShininess = shaderList.at(0)->getShininessLoc();

	glViewport(0, 0, 1366, 768);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glm::vec3 eyePosition = camera.getPosition();
	glUniform3f(uniformEyePosition, eyePosition.x, eyePosition.y, eyePosition.z);

	shaderList.at(0)->setDirectionalLight(&mainLight);
	shaderList.at(0)->setPointLights(pointLights, pointLightCount);
	shaderList.at(0)->setSpotLights(spotLights, spotLightCount);
	glm::mat4 aux = mainLight.computeLightTransform();
	shaderList.at(0)->setDirectionalLightTransform(&aux);

	mainLight.getShadowMap()->read(GL_TEXTURE1);
	shaderList.at(0)->setTexture(0);
	shaderList.at(0)->setDirectionalShadowMap(1);

	glm::vec3 flashLightPosition = camera.getPosition();
	flashLightPosition.y -= 0.1f;
	//spotLights[0].setFlash(flashLightPosition, camera.getDirection());

	renderScene();
}

void directionalShadowMapPass(DirectionalLight* light) {
	directionalShadowShader.useShader();
	glViewport(0, 0, light->getShadowMap()->getShadowWidth(), light->getShadowMap()->getShadowHeight());
	light->getShadowMap()->write();
	glClear(GL_DEPTH_BUFFER_BIT);
	uniformModel = directionalShadowShader.getModelLoc();
	glm::mat4 aux = light->computeLightTransform();
	directionalShadowShader.setDirectionalLightTransform(&aux);
	renderScene();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main() {
	mainWindow = Window(1366, 768);
	if(mainWindow.initialise("GKS") < 0)
		return -1;

	createObjects();
	createShaders();

	camera = Camera(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, -90.0f, 0.0f, 5.0f, 0.1f);

	brickTex = Texture("models/textures/brick.png");
	brickTex.loadTextureA();
	dirtTex = Texture("models/textures/dirt.png");
	dirtTex.loadTextureA();
	plainTex = Texture("models/textures/plain.png");
	plainTex.loadTextureA();



	mainLight = DirectionalLight(
		1024, 1024,
		1.0f, 1.0f, 1.0f,
		0.4f, 0.3f,
		0.0f, -15.0f, -10.0f);
	pointLights[pointLightCount++] = PointLight(
		0.0f, 0.0f, 1.0f,
		0.0f, 0.1f,
		0.0f, 0.0f, 0.0f,
		0.3f, 0.2f, 0.1f);
	pointLights[pointLightCount++] = PointLight(
		0.0f, 1.0f, 0.0f,
		0.0f, 0.1f,
		-4.0f, 2.0f, 0.0f,
		0.3f, 0.1f, 0.1f);
	spotLights[spotLightCount++] = SpotLight(
		1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		20.0f);
	spotLights[spotLightCount++] = SpotLight(
		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, -1.5f, 0.0f,
		-100.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		20.0f);

	shiny = Material(1.0f, 32.0f);
	dull = Material(0.3f, 4.0f);

	xWing = Model();
	xWing.loadModel("models/x-wing/x-wing.obj");
	blackHawk = Model();
	blackHawk.loadModel("models/uh60/uh60.obj");

	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(), 0.1f, 100.0f);

	while (!mainWindow.shouldClose()) {
		computeDeltaTime();

		glfwPollEvents();

		camera.keyControl(mainWindow.getKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());		


		directionalShadowMapPass(&mainLight);
		renderPass(projection, camera.computeViewMatrix());

		mainWindow.swapBuffers();
	}
	cleanup();
	return 0;
}