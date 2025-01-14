#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <tuple>
#include <cstdlib>
#include <ctime>

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
Model* tree1{ nullptr }, * jeep{ nullptr }, * deer{ nullptr }, * watchTower{ nullptr }, * goose{ nullptr }, * tiger{ nullptr }, * starWars{ nullptr }, * heli{ nullptr }; //references for cleanup
std::vector<std::tuple<Model*, glm::vec3*, glm::vec3*, glm::vec3*>> trees; //model, translation, scale, rotation
std::vector<std::tuple<Model*, glm::vec3*, glm::vec3*, glm::vec3*>> deerS; //model, translation, scale, rotation
std::vector<std::tuple<Model*, glm::vec3*, glm::vec3*, glm::vec3*>> geese; //model, translation, scale, rotation

Material shiny, dull;
Texture brickTex, dirtTex, plainTex;

static void createTrees();
static void createDeerS();
static void createGeese();
static void createWatchTower();
static void createSceneFloor();
static void createTiger();
static void createJeep();
static void createStarWars();
static void createHeli();

static void renderModels(std::vector<std::tuple<Model*, glm::vec3*, glm::vec3*, glm::vec3*>> modelList, Material material);
static void renderFloor();
static void renderWatchTower();
static void renderTiger();
static void renderJeep();
static void renderStarWars();
static void renderHeli();

static void computeDeltaTime();
static void cleanup();
static float randomFloat();

static void createObjects()
{	
	createSceneFloor();
	createTrees();
	createDeerS();
	createGeese();
	createWatchTower();
	createTiger();
	createJeep();
	createStarWars();
	createHeli();
}

static void createShaders() {
	Shader* shader = new Shader();
	shader->createFromFiles("shaders/main.vert", "shaders/main.frag");
	shaderList.push_back(shader);

	directionalShadowShader.createFromFiles("shaders/directionalShadow.vert", "shaders/directionalShadow.frag");

	omniShadowShader.createFromFiles("shaders/omnishadowMap.vert", "shaders/omnishadowMap.geom", "shaders/omnishadowMap.frag");
}

float heliAngle = 0.0f;

static void renderScene() {
	// floor
	renderFloor();
	renderModels(trees, shiny);
	renderModels(deerS, dull);
	renderModels(geese, dull);
	renderWatchTower();
	renderTiger();
	renderJeep();
	renderStarWars();
	renderHeli();
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

	//glm::vec3 flashLightPosition = camera.getPosition();
	//flashLightPosition.y -= 0.1f;
	//spotLights[0].setFlash(flashLightPosition, camera.getDirection());

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
	srand(time(NULL));
	mainWindow = Window(1366, 768);
	if(mainWindow.initialise("GKS") < 0)
		return -1;

	createObjects();
	createShaders();

	camera = Camera(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, -90.0f, 0.0f, 5.0f, 0.1f);

	brickTex = Texture("models/textures/brick.png");
	brickTex.loadTextureA();
	dirtTex = Texture("models/textures/dirt.jpg");
	dirtTex.loadTexture();
	plainTex = Texture("models/textures/plain.png");
	plainTex.loadTextureA();

	mainLight = DirectionalLight(
		2048, 2048,
		1.0f, 1.0f, 1.0f,
		0.5f, 0.1f,
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
		0.8f, 1.0f, 1.0f,
		0.8f, 2.0f,
		15.0f, 1.0f, 16.565f,
		-0.05f, -0.8f, -0.9f,
		0.3f, 0.2f, 0.1f,
		20.0f);
	spotLights[spotLightCount++] = SpotLight(
		1024, 1024,
		0.1f, 100.0f,
		1.0f, 0.8f, 1.0f,
		0.8f, 1.2f,
		14.6f, 1.0f, 17.2f,
		-0.3f, -0.8f, -0.9f,
		1.0f, 0.0f, 0.0f,
		20.0f);
	pointLightCount = 0;
	spotLightCount = 0;

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

	

	glm::mat4 projection = glm::perspective(glm::radians(60.0f), (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(), 0.1f, 100.0f);
	int doShadows = 0;

	while (!mainWindow.shouldClose()) {
		computeDeltaTime();

		glfwPollEvents();

		camera.keyControl(mainWindow.getKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());		

		if (mainWindow.getKeys()[GLFW_KEY_L]) {
			spotLights[0].toggle();
			mainWindow.unsetKey(GLFW_KEY_L);
		}
		//if (doShadows == 0) {
			directionalShadowMapPass(&mainLight);
		//}
		//else if(doShadows == 1){
			for (size_t i = 0; i < pointLightCount; i++)
				omnidirectionalShadowMapPass(&pointLights[i]);
			for (size_t i = 0; i < spotLightCount; i++)
				omnidirectionalShadowMapPass(&spotLights[i]);
		//}
		//else
			renderPass(projection, camera.computeViewMatrix());
		doShadows = (doShadows + 1) % 3;
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
	plainTex.useTexture();
	//dirtTex.useTexture();
	dull.useMaterial(uniformSpecularI, uniformShininess);
	sceneFloor->renderMesh();
}

void renderWatchTower()
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(6.0f, WATCHTOWER_Y_TRANSLATION, 5.0f));
	model = glm::scale(model, glm::vec3(WATCHTOWER_SCALE_FACTOR));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dull.useMaterial(uniformSpecularI, uniformShininess);
	watchTower->renderModel();
}

void renderTiger()
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(13.0f, TIGER_Y_TRANSLATION, -14.0f));
	model = glm::scale(model, glm::vec3(TIGER_SCALE_FACTOR));
	model = glm::rotate(model, glm::radians(2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dull.useMaterial(uniformSpecularI, uniformShininess);
	tiger->renderModel();
}

void renderJeep()
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(15.0f, -1.55f, 16.0f));
	model = glm::scale(model, glm::vec3(1.0f));
	model = glm::rotate(model, glm::radians(200.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dull.useMaterial(uniformSpecularI, uniformShininess);
	jeep->renderModel();
}

float starWarsX = -20.0f;
float acc = 0.0f;
float x = -0.003f;

void renderStarWars()
{
	acc = powf(x, 2) * 5000;
	starWarsX += 0.01f + acc;
	x += 0.00001f;
	if (starWarsX > 20.0f) {
		starWarsX = -20.0f;
		x = -0.003f;
	}
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-starWarsX - 8.0f, 0.0f, starWarsX));
	model = glm::scale(model, glm::vec3(0.006f, 0.006f, 0.006f));
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3{ 0.0f, 1.0f, 0.0f });
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	shiny.useMaterial(uniformSpecularI, uniformShininess);
	starWars->renderModel();
}

void renderHeli()
{
	heliAngle += 0.1f;
	if (heliAngle > 360.0f)
	{
		heliAngle = 0.1f;
	}

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3{ 5.5f, 0.0f, 5.5f });
	model = glm::rotate(model, glm::radians(-heliAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-4.0f, 5.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	shiny.useMaterial(uniformSpecularI, uniformShininess);
	heli->renderModel();
}

static void renderModels(std::vector<std::tuple<Model*, glm::vec3*, glm::vec3*, glm::vec3*>> modelList, Material material)
{
	for (auto& i : modelList) {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, *std::get<1>(i));
		model = glm::scale(model, (*std::get<2>(i)));
		model = glm::rotate(model, (*std::get<3>(i)).x, glm::vec3{ 1.0f, 0.0f, 0.0f });
		model = glm::rotate(model, (*std::get<3>(i)).y, glm::vec3{ 0.0f, 1.0f, 0.0f });
		model = glm::rotate(model, (*std::get<3>(i)).z, glm::vec3{ 0.0f, 0.0f, 1.0f });
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dirtTex.useTexture();
		material.useMaterial(uniformSpecularI, uniformShininess);
		std::get<0>(i)->renderModel();
	}
}

static void createTrees() 
{
	tree1 = new Model();
	tree1->loadModel("models/tree1/trunk wood.obj");

	//tree2 = new Model();
	//tree2->loadModel("models/tree2/bush.obj");

	float randomScaleF = randomFloat();
	trees.push_back({
		tree1,
		new glm::vec3{5.0f, LOW_POLY_Y_TRANSLATION, -2.0f},
		new glm::vec3{ randomScaleF * LOW_POLY_TREE1_SCALE_FACTOR },
		new glm::vec3{0.0f, 1.45f, 0.0f} });

	randomScaleF = randomFloat();
	trees.push_back({
		tree1,
		new glm::vec3{2.0f, LOW_POLY_Y_TRANSLATION, -10.0f},
		new glm::vec3{ randomScaleF * LOW_POLY_TREE1_SCALE_FACTOR},
		new glm::vec3{0.0f, -1.23f, 0.0f} });

	randomScaleF = randomFloat();
	trees.push_back({
		tree1,
		new glm::vec3{10.0f, LOW_POLY_Y_TRANSLATION, -12.0f},
		new glm::vec3{ randomScaleF * LOW_POLY_TREE1_SCALE_FACTOR},
		new glm::vec3{0.0f, 3.78f, 0.0f} });

	randomScaleF = randomFloat();
	trees.push_back({
		tree1,
		new glm::vec3{-13.0f, LOW_POLY_Y_TRANSLATION, 5.0f},
		new glm::vec3{ randomScaleF * LOW_POLY_TREE1_SCALE_FACTOR},
		new glm::vec3{0.0f, -5.98f, 0.0f} });

	randomScaleF = randomFloat();
	trees.push_back({
		tree1,
		new glm::vec3{-3.0f, LOW_POLY_Y_TRANSLATION, 9.0f},
		new glm::vec3{ randomScaleF * LOW_POLY_TREE1_SCALE_FACTOR},
		new glm::vec3{0.0f, 5.45f, 0.0f} });

	randomScaleF = randomFloat();
	trees.push_back({
		tree1,
		new glm::vec3{-5.0f, LOW_POLY_Y_TRANSLATION, -6.0f},
		new glm::vec3{ randomScaleF * LOW_POLY_TREE1_SCALE_FACTOR},
		new glm::vec3{0.0f, -2.34f, 0.0f} });

	randomScaleF = randomFloat();
	trees.push_back({
		tree1,
		new glm::vec3{15.0f, LOW_POLY_Y_TRANSLATION, 4.0f},
		new glm::vec3{ randomScaleF * LOW_POLY_TREE1_SCALE_FACTOR},
		new glm::vec3{0.0f, 6.23f, 0.0f} });

	randomScaleF = randomFloat();
	trees.push_back({
		tree1,
		new glm::vec3{5.0f, LOW_POLY_Y_TRANSLATION, 13.0f},
		new glm::vec3{ randomScaleF * LOW_POLY_TREE1_SCALE_FACTOR},
		new glm::vec3{0.0f, 2.34f, 0.0f} });

	/*randomScaleF = randomFloat();
	trees.push_back({
		tree2,
		new glm::vec3{19.0f, LOW_POLY_Y_TRANSLATION, -8.0f},
		new glm::vec3{ randomScaleF * LOW_POLY_TREE2_SCALE_FACTOR},
		new glm::vec3{0.0f, 1.34f, 0.0f} });

	randomScaleF = randomFloat();
	trees.push_back({
		tree2,
		new glm::vec3{7.4f, LOW_POLY_Y_TRANSLATION, -3.5f},
		new glm::vec3{ randomScaleF * LOW_POLY_TREE2_SCALE_FACTOR},
		new glm::vec3{0.0f, -1.78f, 0.0f} });

	randomScaleF = randomFloat();
	trees.push_back({
		tree2,
		new glm::vec3{-15.0f, LOW_POLY_Y_TRANSLATION, -7.0f},
		new glm::vec3{ randomScaleF * LOW_POLY_TREE2_SCALE_FACTOR},
		new glm::vec3{0.0f, 0.21f, 0.0f} });*/

}
static void createDeerS() 
{
	deer = new Model();
	deer->loadModel("models/deer/79377.obj");

	deerS.push_back({
		deer,
		new glm::vec3{-10.0f, DEER_Y_TRANSLATION, -5.0f},
		new glm::vec3{DEER_SCALE_FACTOR},
		new glm::vec3{glm::radians(-90.0f), 0.0f, 0.0f}});

	deerS.push_back({
		deer,
		new glm::vec3{-9.0f, DEER_Y_TRANSLATION, -7.0f},
		new glm::vec3{DEER_SCALE_FACTOR},
		new glm::vec3{glm::radians(-90.0f), 0.0f, 0.0f} });

	deerS.push_back({
		deer,
		new glm::vec3{-11.0f, DEER_Y_TRANSLATION, -3.1f},
		new glm::vec3{DEER_SCALE_FACTOR},
		new glm::vec3{glm::radians(-90.0f), 0.0f, 0.0f} });

	deerS.push_back({
		deer,
		new glm::vec3{-10.3f, DEER_Y_TRANSLATION, -6.2f},
		new glm::vec3{DEER_SCALE_FACTOR},
		new glm::vec3{glm::radians(-90.0f), 0.0f, 0.0f} });

	deerS.push_back({
		deer,
		new glm::vec3{-8.4f, DEER_Y_TRANSLATION, -6.3f},
		new glm::vec3{DEER_SCALE_FACTOR},
		new glm::vec3{glm::radians(-90.0f), 0.0f, 0.0f} });
}
void createGeese()
{
	goose = new Model();
	goose->loadModel("models/goose/goose.obj");

	geese.push_back({
		goose,
		new glm::vec3{0.3f, GOOSE_Y_TRANSLATION, 1.0f},
		new glm::vec3{},
		new glm::vec3{glm::radians(0.0f), glm::radians(randomFloat() * 100.0f), 0.0f} });
	geese.push_back({
		goose,
		new glm::vec3{0.6f, GOOSE_Y_TRANSLATION, 2.3f},
		new glm::vec3{GOOSE_SCALE_FACTOR},
		new glm::vec3{glm::radians(0.0f), glm::radians(randomFloat() * 100.0f), 0.0f} });
	geese.push_back({
		goose,
		new glm::vec3{0.8f, GOOSE_Y_TRANSLATION, -1.5f},
		new glm::vec3{GOOSE_SCALE_FACTOR},
		new glm::vec3{glm::radians(0.0f), glm::radians(randomFloat() * 100.0f), 0.0f} });
	geese.push_back({
		goose,
		new glm::vec3{0.2f, GOOSE_Y_TRANSLATION, -0.4f},
		new glm::vec3{GOOSE_SCALE_FACTOR},
		new glm::vec3{glm::radians(0.0f), glm::radians(randomFloat() * 100.0f), 0.0f} });
	geese.push_back({
		goose,
		new glm::vec3{-0.6f, GOOSE_Y_TRANSLATION, 0.6f},
		new glm::vec3{GOOSE_SCALE_FACTOR},
		new glm::vec3{glm::radians(0.0f), glm::radians(randomFloat() * 100.0f), 0.0f} });
	geese.push_back({
		goose,
		new glm::vec3{-1.0f, GOOSE_Y_TRANSLATION, 1.4f},
		new glm::vec3{GOOSE_SCALE_FACTOR},
		new glm::vec3{glm::radians(0.0f), glm::radians(randomFloat() * 100.0f), 0.0f} });
	geese.push_back({
		goose,
		new glm::vec3{-2.0f, GOOSE_Y_TRANSLATION, 1.9f},
		new glm::vec3{GOOSE_SCALE_FACTOR},
		new glm::vec3{glm::radians(0.0f), glm::radians(randomFloat() * 100.0f), 0.0f} });
	geese.push_back({
		goose,
		new glm::vec3{1.0f, GOOSE_Y_TRANSLATION, 2.1f},
		new glm::vec3{GOOSE_SCALE_FACTOR},
		new glm::vec3{glm::radians(0.0f), glm::radians(randomFloat() * 100.0f), 0.0f} });
}
void createWatchTower()
{
	watchTower = new Model();
	watchTower->loadModel("models/watchtower/wooden_watch_tower2.obj");

}
void createSceneFloor()
{
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
}
void createTiger()
{
	tiger = new Model();
	tiger->loadModel("models/tiger/B0715165.obj");
}
void createJeep()
{
	jeep = new Model();
	jeep->loadModel("models/jeep/78765.obj");
}

void createStarWars()
{
	starWars = new Model();
	starWars->loadModel("models/x-wing/x-wing.obj");
}

void createHeli()
{
	heli = new Model();
	heli->loadModel("models/uh60/uh60.obj");
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
	for (auto& i : trees) {
		delete std::get<1>(i);
		delete std::get<2>(i);
		delete std::get<3>(i);
	}
	for (auto& i : deerS) {
		delete std::get<1>(i);
		delete std::get<2>(i);
		delete std::get<3>(i);
	}
	for (auto& i : geese) {
		delete std::get<1>(i);
		delete std::get<2>(i);
		delete std::get<3>(i);
	}
	if(sceneFloor)
		delete sceneFloor;
	if (tree1)
		delete tree1;
	if (jeep)
		delete jeep;
	if (deer)
		delete deer;
	if (watchTower)
		delete watchTower;
	if (goose)
		delete goose;
	if (tiger)
		delete tiger;
	if (starWars)
		delete starWars;
}

static float randomFloat()
{
	float aux = ((float)(rand() % 5 + 1) / (float)(rand() % 5 + 2));
	return (aux > 3.5f) ? 
		1.0f : 
		(aux < 0.7f ) ? 
		1.0f :
		aux;
}