#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <tuple>

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
#include "SkyBox.hpp"

Window mainWindow;
Camera camera;
DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];
std::vector<Shader*> shaderList;
Shader directionalShadowShader;
Shader omniShadowShader;
SkyBox skybox{};

GLuint uniformProjection{ 0 }, uniformModel{ 0 }, uniformView{ 0 };
GLuint uniformEyePosition{ 0 }, uniformShininess{ 0 }, uniformSpecularI{ 0 };
GLuint uniformOmniLightPos{ 0 }, uniformFarPlane{ 0 };

GLfloat deltaTime{ 0.0f };
GLfloat lastTime{ 0.0f };

unsigned int pointLightCount = 0;
unsigned int spotLightCount = 0;

Mesh* sceneFloor{ nullptr };
std::vector<Mesh*> meshList;
std::vector<std::tuple<Model*, glm::vec3*, glm::vec3*, glm::vec3*>> trees; //model, translation, scale, rotation

Material shiny, dull;
Texture brickTex, dirtTex, plainTex;
Model xWing, blackHawk, quad;

static void createTrees();

static void renderTrees();
static void renderFloor();

static void computeDeltaTime();
static void cleanup();

static void createObjects() {
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

	sceneFloor = new Mesh();
	sceneFloor->createMesh(floorVertices, floorIndices, 32, 6);
	
	createTrees();
}

static void createShaders() {
	Shader* shader = new Shader();
	shader->createFromFiles("shaders/main.vert", "shaders/main.frag");
	shaderList.push_back(shader);

	directionalShadowShader.createFromFiles("shaders/directionalShadow.vert", "shaders/directionalShadow.frag");

	omniShadowShader.createFromFiles("shaders/omnishadowMap.vert", "shaders/omnishadowMap.geom", "shaders/omnishadowMap.frag");
}

float blackhawkAngle = 0.0f;

static void renderScene() {
	// floor
	renderFloor();

	renderTrees();

	/*glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-7.0f, 0.0f, 10.0f));
	model = glm::scale(model, glm::vec3(0.006f, 0.006f, 0.006f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	shiny.useMaterial(uniformSpecularI, uniformShininess);
	xWing.renderModel();

	blackhawkAngle += 0.1f;
	if (blackhawkAngle > 360.0f)
	{
		blackhawkAngle = 0.1f;
	}

	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(-blackhawkAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-8.0f, 2.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	shiny.useMaterial(uniformSpecularI, uniformShininess);
	blackHawk.renderModel();*/
}

static void renderPass(glm::mat4 projectionMatrix, glm::mat4 viewMatrix) 
{
	glViewport(0, 0, 1366, 768);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	skybox.draw(viewMatrix, projectionMatrix);

	shaderList.at(0)->useShader();

	uniformModel = shaderList.at(0)->getModelLoc();
	uniformProjection = shaderList.at(0)->getProjectionLoc();
	uniformView = shaderList.at(0)->getViewLoc();
	uniformEyePosition = shaderList.at(0)->getEyePositionLoc();
	uniformSpecularI = shaderList.at(0)->getSpecularILoc();
	uniformShininess = shaderList.at(0)->getShininessLoc();

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glm::vec3 eyePosition = camera.getPosition();
	glUniform3f(uniformEyePosition, eyePosition.x, eyePosition.y, eyePosition.z);

	shaderList.at(0)->setDirectionalLight(&mainLight);
	shaderList.at(0)->setPointLights(pointLights, pointLightCount, 3, 0);
	shaderList.at(0)->setSpotLights(spotLights, spotLightCount, 3 + pointLightCount, pointLightCount);
	glm::mat4 aux = mainLight.computeLightTransform();
	shaderList.at(0)->setDirectionalLightTransform(&aux);

	mainLight.getShadowMap()->read(GL_TEXTURE2);
	shaderList.at(0)->setTexture(1);
	shaderList.at(0)->setDirectionalShadowMap(2);

	glm::vec3 flashLightPosition = camera.getPosition();
	flashLightPosition.y -= 0.1f;
	spotLights[0].setFlash(flashLightPosition, camera.getDirection());

	shaderList.at(0)->validate();

	renderScene();
}

static void directionalShadowMapPass(DirectionalLight* light) {
	directionalShadowShader.useShader();
	glViewport(0, 0, light->getShadowMap()->getShadowWidth(), light->getShadowMap()->getShadowHeight());

	light->getShadowMap()->write();
	glClear(GL_DEPTH_BUFFER_BIT);

	uniformModel = directionalShadowShader.getModelLoc();
	glm::mat4 aux = light->computeLightTransform();
	directionalShadowShader.setDirectionalLightTransform(&aux);

	directionalShadowShader.validate();

	renderScene();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void omnidirectionalShadowMapPass(PointLight* light) {
	omniShadowShader.useShader();
	glViewport(0, 0, light->getShadowMap()->getShadowWidth(), light->getShadowMap()->getShadowHeight());

	light->getShadowMap()->write();
	glClear(GL_DEPTH_BUFFER_BIT);

	uniformModel = omniShadowShader.getModelLoc();
	uniformOmniLightPos = omniShadowShader.getOmniLightPosLoc();
	uniformFarPlane = omniShadowShader.getFarPlanePos();

	glm::vec3 pos = light->getPosition();
	glUniform3f(uniformOmniLightPos, pos.x, pos.y, pos.z);

	GLfloat f = light->getFarPlane();
	glUniform1f(uniformFarPlane, f);

	std::vector<glm::mat4> lightMs = light->computeLightTransform();
	omniShadowShader.setOmniLightMatrices(lightMs);

	omniShadowShader.validate();

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
		2048, 2048,
		1.0f, 1.0f, 1.0f,
		0.00f, 0.0f,
		0.0f, -15.0f, -10.0f);
	pointLights[pointLightCount++] = PointLight(
		1024, 1024,
		0.1f, 100.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.4f,
		1.0f, 2.0f, 0.0f,
		0.3f, 0.01f, 0.01f);
	pointLights[pointLightCount++] = PointLight(
		1024, 1024,
		0.1f, 100.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.4f,
		-4.0f, 3.0f, 0.0f,
		0.3f, 0.01f, 0.01f);
	spotLights[spotLightCount++] = SpotLight(
		1024, 1024,
		0.1f, 100.0f,
		1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		20.0f);
	spotLights[spotLightCount++] = SpotLight(
		1024, 1024,
		0.1f, 100.0f,
		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, -1.5f, 0.0f,
		-100.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		20.0f);

	skybox = (SKY_BOX_TYPE == 0) ?
		SkyBox({
		"models/textures/skybox/posx.jpg",
		"models/textures/skybox/negx.jpg",
		"models/textures/skybox/posy.jpg",
		"models/textures/skybox/negy.jpg",
		"models/textures/skybox/posz.jpg",
		"models/textures/skybox/negz.jpg" })
		:
		SkyBox({
		"models/textures/skybox2/cupertin-lake_rt.tga",
		"models/textures/skybox2/cupertin-lake_lf.tga",
		"models/textures/skybox2/cupertin-lake_up.tga",
		"models/textures/skybox2/cupertin-lake_dn.tga",
		"models/textures/skybox2/cupertin-lake_bk.tga",
		"models/textures/skybox2/cupertin-lake_ft.tga" });

	shiny = Material(1.0f, 32.0f);
	dull = Material(0.3f, 4.0f);

	xWing = Model();
	xWing.loadModel("models/x-wing/x-wing.obj");
	blackHawk = Model();
	blackHawk.loadModel("models/uh60/uh60.obj");

	glm::mat4 projection = glm::perspective(glm::radians(60.0f), (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(), 0.1f, 100.0f);

	while (!mainWindow.shouldClose()) {
		computeDeltaTime();

		glfwPollEvents();

		camera.keyControl(mainWindow.getKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());		

		if (mainWindow.getKeys()[GLFW_KEY_L]) {
			spotLights[0].toggle();
			mainWindow.unsetKey(GLFW_KEY_L);
		}

		directionalShadowMapPass(&mainLight);
		for (size_t i = 0; i < pointLightCount; i++)
			omnidirectionalShadowMapPass(&pointLights[i]);
		for (size_t i = 0; i < spotLightCount; i++)
			omnidirectionalShadowMapPass(&spotLights[i]);
		renderPass(projection, camera.computeViewMatrix());

		mainWindow.swapBuffers();
	}
	cleanup();
	return 0;
}

static void renderFloor() {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dirtTex.useTexture();
	shiny.useMaterial(uniformSpecularI, uniformShininess);
	sceneFloor->renderMesh();
}

static void renderTrees()
{
	for (auto& i : trees) {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, *std::get<1>(i));
		model = glm::scale(model, *std::get<2>(i));
		model = glm::rotate(model, (*std::get<3>(i)).x, glm::vec3{ 1.0f, 0.0f, 0.0f });
		model = glm::rotate(model, (*std::get<3>(i)).y, glm::vec3{ 0.0f, 1.0f, 0.0f });
		model = glm::rotate(model, (*std::get<3>(i)).z, glm::vec3{ 0.0f, 0.0f, 1.0f });
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		shiny.useMaterial(uniformSpecularI, uniformShininess);
		std::get<0>(i)->renderModel();
	}
}

static void createTrees() 
{
	Model* tree = new Model();
	tree->loadModel("models/tree/Lowpoly_tree_sample.obj");

	trees.push_back({
		tree,
		new glm::vec3{5.0f, LOW_POLY_Y_TRANSLATION, -2.0f},
		new glm::vec3{LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR},
		new glm::vec3{0.0f, 1.45f, 0.0f} });
	trees.push_back({
		tree,
		new glm::vec3{2.0f, LOW_POLY_Y_TRANSLATION, -10.0f},
		new glm::vec3{LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR},
		new glm::vec3{0.0f, -1.23f, 0.0f} });
	trees.push_back({
		tree,
		new glm::vec3{10.0f, LOW_POLY_Y_TRANSLATION, -12.0f},
		new glm::vec3{LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR},
		new glm::vec3{0.0f, 3.78f, 0.0f} });
	trees.push_back({
		tree,
		new glm::vec3{-13.0f, LOW_POLY_Y_TRANSLATION, 5.0f},
		new glm::vec3{LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR},
		new glm::vec3{0.0f, -0.98f, 0.0f} });
	trees.push_back({
		tree,
		new glm::vec3{-3.0f, LOW_POLY_Y_TRANSLATION, 9.0f},
		new glm::vec3{LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR},
		new glm::vec3{0.0f, 0.45f, 0.0f} });
	trees.push_back({
		tree,
		new glm::vec3{-5.0f, LOW_POLY_Y_TRANSLATION, -6.0f},
		new glm::vec3{LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR},
		new glm::vec3{0.0f, -2.34f, 0.0f} });
	trees.push_back({
		tree,
		new glm::vec3{15.0f, LOW_POLY_Y_TRANSLATION, 4.0f},
		new glm::vec3{LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR},
		new glm::vec3{0.0f, 1.23f, 0.0f} });
	trees.push_back({
		tree,
		new glm::vec3{5.0f, LOW_POLY_Y_TRANSLATION, 13.0f},
		new glm::vec3{LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR, LOW_POLY_SCALE_FACTOR},
		new glm::vec3{0.0f, 2.34f, 0.0f} });

}

static void computeDeltaTime() 
{
	GLfloat currTime = (GLfloat)glfwGetTime();
	deltaTime = currTime - lastTime;
	lastTime = currTime;
}

static void cleanup() 
{
	for (auto& i : meshList)
		delete i;
	for (auto& i : shaderList)
		delete i;
	//remove the resource that all trees point to
	if(trees.size() > 0) 
		delete std::get<0>(trees.at(0));
	for (auto& i : trees) {
		delete std::get<1>(i);
		delete std::get<2>(i);
		delete std::get<3>(i);
	}
	delete sceneFloor;
}