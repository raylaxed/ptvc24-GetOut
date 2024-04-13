/*
* Copyright 2021 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/


#include "Utils.h"
#include <sstream>
#include <fstream>
#include "Camera.h"
#include "Shader.h"
#include "Geometry.h"
#include "Material.h"
#include "Light.h"
#include "Texture.h"
#include "Player.h"
#include "PhysicsWorld.h"
#include <glm/gtx/string_cast.hpp>
#include "Text.h"
#include "Timer.h"
#include "Model.h"
#include <iostream>
#include "ParticleSystem.h"






/* --------------------------------------------- */
// Prototypes
/* --------------------------------------------- */
void drawPSVector(std::vector<ParticleSystem*> x, float deltaTime, float newparticles);
void drawModelVector(Model* model, std::vector<glm::mat4*> x);
void drawGeometryVector(std::vector<Geometry*> x);
void setPerFrameUniforms(Shader* shader, Camera& camera, glm::mat4 projMatrix, PointLight& pointL);
void setPerFrameUniforms(Shader* shader, Camera& camera, glm::mat4 projMatrix, PointLight& pointL, int lightID);
void setPerFrameUniforms(Shader* shader, Camera& camera, glm::mat4 projMatrix, DirectionalLight& dirL, glm::vec3 color);
static void APIENTRY DebugCallbackDefault(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void setPerFrameUniforms(Shader* shader, Camera& camera, glm::mat4 projMatrix, DirectionalLight& dirL, PointLight& pointL);
void processKeyInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double x, double y);
void drawTrapsOrLava(std::vector<Geometry*> x, boolean isTrap = true);
void dashCooldownDraw(Shader& shader);
void postProcessing(shared_ptr<Shader> blurShader, shared_ptr<Shader> bloomShader, bool bloom, float exposure);
void createQuad();
void createTrapHitbox(glm::vec3 position, float size, shared_ptr<Material>);
std::vector<glm::mat4*> createTorches(Model torch, std::shared_ptr<Material> mat);
std::vector<PointLight*> createLights(glm::vec3 flamecolor);
std::vector<glm::mat4*> createBulbs();

/* --------------------------------------------- */
// Global variables
/* --------------------------------------------- */

static bool _wireframe = false;
static bool _culling = true;
static bool _dragging = false;
static bool _strafing = false;
static float _zoom = 6.0f;
//time between frame and its predecesor
float deltaTime = 0.0f; 
float lastFrame = 0.0f;

//GameLogic bool
bool resetGame = false;
bool isDead = false;
int highscore = 0;


// Initialize camera
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
float lastX = 0;
float lastY = 0;
bool firstMouse = true;
glm::vec3 pos = camera.getPosition();

Player player = Player(camera);
PhysicsWorld* pWorld = new PhysicsWorld();
std::map<GLchar, Character> _charactersForCooldown;
std::map<GLchar, Character> _characters;

//global buffers
unsigned int VAO, VBO;
unsigned int pingpongFBO[2];
unsigned int pingpongColorbuffers[2];
unsigned int colorBuffers[2];

unsigned int qVAO = 0;
unsigned int qVBO;
int window_width;
int window_height;

/* --------------------------------------------- */
// Main
/* --------------------------------------------- */

int main(int argc, char** argv)
{
	/* --------------------------------------------- */
	// Load settings.ini
	/* --------------------------------------------- */


	
	INIReader reader("assets/settings.ini");

	window_width = reader.GetInteger("window", "width", 800);
	window_height = reader.GetInteger("window", "height", 800);
	lastX = (float)window_width;
	lastY = (float)window_height;
	int refresh_rate = reader.GetInteger("window", "refresh_rate", 60);
	bool fullscreen = reader.GetBoolean("window", "fullscreen", false);
	std::string window_title = reader.Get("window", "title", "Get Out!");
	float fov = float(reader.GetReal("camera", "fov", 60.0f));
	float nearZ = float(reader.GetReal("camera", "near", 0.1f));
	float farZ = float(reader.GetReal("camera", "far", 100.0f));


	/* --------------------------------------------- */
	// Create context
	/* --------------------------------------------- */

	glfwSetErrorCallback([](int error, const char* description) { std::cout << "GLFW error " << error << ": " << description << std::endl; });

	if (!glfwInit()) {
		EXIT_WITH_ERROR("Failed to init GLFW");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // Request OpenGL version 4.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Request core profile
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);  // Create an OpenGL debug context 
	glfwWindowHint(GLFW_REFRESH_RATE, refresh_rate); // Set refresh rate
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// Enable antialiasing (4xMSAA)
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Open window
	GLFWmonitor* monitor = nullptr;

	if (fullscreen)
		monitor = glfwGetPrimaryMonitor();

	GLFWwindow* window = glfwCreateWindow(window_width, window_height, window_title.c_str(), monitor, nullptr);

	if (!window) EXIT_WITH_ERROR("Failed to create window");


	// This function makes the context of the specified window current on the calling thread. 
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true;
	GLenum err = glewInit();

	// If GLEW wasn't initialized
	if (err != GLEW_OK) {
		EXIT_WITH_ERROR("Failed to init GLEW: " << glewGetErrorString(err));
	}

	// Debug callback
	if (glDebugMessageCallback != NULL) {
		// Register your callback function.

		glDebugMessageCallback(DebugCallbackDefault, NULL);
		// Enable synchronous callback. This ensures that your callback function is called
		// right after an error has occurred. This capability is not defined in the AMD
		// version.
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}


	/* --------------------------------------------- */
	// Init framework + text shader and freetype
	/* --------------------------------------------- */

	//TEXT SHADERS IMPLEMENT LIKE IN SHADER.h
	
	pWorld->initPhysics();
	PxScene* pScene = pWorld->getScene();

	if (!initFramework()) {
		EXIT_WITH_ERROR("Failed to init framework");
	}

	// set callbacks
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	// set GL defaults
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	/* --------------------------------------------- */
	// Initialize scene and render loop
	/* --------------------------------------------- */
	{


		// Load shader(s)
		std::shared_ptr<Shader> textureShader = std::make_shared<Shader>("texture.vert", "cook_torrance.frag");
		std::shared_ptr<Shader> particleShader = std::make_shared<Shader>("particle.vert", "particle.frag");
		std::shared_ptr<Shader> cookTexturedShader = std::make_shared<Shader>("textureCook.vert", "cook_torrance_textured.frag");
		std::shared_ptr<Shader> animationShader = std::make_shared<Shader>("animation.vert", "cook_torrance.frag");
		std::shared_ptr<Shader> hudShader = std::make_shared<Shader>("hud.vert", "hud.frag");
		
		std::shared_ptr<Shader> lightMakerShader = std::make_shared<Shader>("textureCook.vert", "visible_light_maker.frag");
		std::shared_ptr<Shader> blurShader = std::make_shared<Shader>("blur.vert", "blur.frag");
		std::shared_ptr<Shader> bloomShader = std::make_shared<Shader>("bloom.vert", "bloom.frag");
		

		glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(window_width), 0.0f, static_cast<float>(window_height));
		hudShader->use();
		hudShader->setUniform("projection", projection);
		hudShader->initFreeType(_charactersForCooldown, "assets/arial.ttf");
		hudShader->initFreeType(_characters, "assets/green_fuz.otf");

		// Create textures 
	
		std::shared_ptr<Texture> groundTexture = std::make_shared<Texture>("Jute_cocomat_pxr128.dds");
		std::shared_ptr<Texture> wallTexture = std::make_shared<Texture>("wall.dds");
		
		std::shared_ptr<Texture> lavaTexture = std::make_shared<Texture>("lava.dds");
		std::shared_ptr<Texture> particle = std::make_shared<Texture>("particles.dds");
		std::shared_ptr<Texture> particleTexture = std::make_shared<Texture>("smoke.dds");

		std::shared_ptr<Texture> groundBase = std::make_shared<Texture>("ground/ground_basecolor.dds");
		std::shared_ptr<Texture> groundAO = std::make_shared<Texture>("ground/ground_ambientocclusion.dds");
		std::shared_ptr<Texture> groundMetallic = std::make_shared<Texture>("ground/ground_metallic.dds");
		std::shared_ptr<Texture> groundRoughness = std::make_shared<Texture>("ground/ground_roughness.dds");
		std::shared_ptr<Texture> groundNormal = std::make_shared<Texture>("ground/ground_normal.dds");
		
		std::shared_ptr<Texture> ballBasecolor = std::make_shared<Texture>("lava/lava_basecolor.dds");
		std::shared_ptr<Texture> ballAO = std::make_shared<Texture>("lava/lava_ambientocclusion.dds");
		std::shared_ptr<Texture> ballMetallic = std::make_shared<Texture>("lava/lava_metallic.dds");
		std::shared_ptr<Texture> ballRoughness = std::make_shared<Texture>("lava/lava_roughness.dds");
		std::shared_ptr<Texture> ballNormal = std::make_shared<Texture>("lava/lava_normal.dds");



		// Create materials,																					x = ambient, y = diffuse, z = specular		
		std::shared_ptr<Material> playerMat = std::make_shared<TextureMaterial>(textureShader, glm::vec3(0.1f, 0.7f, 0.1f), 8.0f, groundBase);
		std::shared_ptr<Material> groundMat = std::make_shared<TextureMaterial>(cookTexturedShader,groundTexture,groundAO,groundMetallic,groundNormal,groundRoughness);
		std::shared_ptr<Material> wallMat = std::make_shared<TextureMaterial>(cookTexturedShader, wallTexture, groundAO, groundMetallic, groundNormal, groundRoughness);
		std::shared_ptr<Material> ballMat = std::make_shared<TextureMaterial>(cookTexturedShader,ballBasecolor,ballAO,ballMetallic,ballRoughness,ballNormal);		
		std::shared_ptr<Material> lavaMat = std::make_shared<TextureMaterial>(animationShader, glm::vec3(0.1f, 0.7f, 0.1f), 8.0f, lavaTexture);

		//load models
		Model torch("assets/objects/torch/textures/untitled.obj",glm::mat4(1.f),*textureShader.get());
		Model armModel("assets/objects/flashlight/scene.gltf",  glm::mat4(1.f), *textureShader.get());
		player.setHand(armModel);

		
		

		glm::vec3 flamecolor = glm::vec3(0.902f, 0.376f, 0.118f);
		
		//create torches
		std::vector<glm::mat4*> bulbs = createBulbs();
		std::vector<PointLight*> pointLights = createLights(flamecolor);
		std::vector<glm::mat4*> torches = createTorches(torch, playerMat);
		
		
		

		// Room
		//initialize floor
		float length = 99.f;
		float width = 99.f;
		

		std::vector<Geometry*> traps;
		std::vector<Geometry*> lava;
		

		Geometry* cube = new Geometry(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 0.0f)), Geometry::createCubeGeometry(1.0f,3.0f, 1.0f), groundMat);
		pWorld->addCubeToPWorld(*cube, glm::vec3(1.0f, 1.0f, 1.0f) * 0.5f);
	
		
		
		//lights
		PointLight* lavaLight = new PointLight(flamecolor, glm::vec3(20, 3.5f, 20), glm::vec3(0.01f));
		pointLights.push_back(lavaLight);
		PointLight* lavaLight2 = new PointLight(flamecolor, glm::vec3(-20, 3.5f, 20), glm::vec3(0.01f));
		pointLights.push_back(lavaLight2);
		PointLight* lavaLight3 = new PointLight(flamecolor, glm::vec3(-20, 3.5f, -20), glm::vec3(0.01f));
		pointLights.push_back(lavaLight3);
		PointLight* lavaLight4 = new PointLight(flamecolor, glm::vec3(20, 3.5f, -20), glm::vec3(0.01f));
		pointLights.push_back(lavaLight4);
		PointLight* lavaLight5 = new PointLight(flamecolor, glm::vec3(0, 3.5f, 0), glm::vec3(0.01f));
		pointLights.push_back(lavaLight5);

		Geometry bulb = Geometry(glm::scale(glm::mat4(1.0f), glm::vec3(1)), Geometry::createSphereGeometry(64, 32, 0.425f), playerMat);
		
		Geometry ground = Geometry(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f)), Geometry::createCubeGeometry(width, 1.f, length), groundMat);
		Geometry wallRight = Geometry(glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, 10.25f, 0.0f)), Geometry::createCubeGeometry(1.f, 50.f, length), wallMat);
		Geometry wallLeft = Geometry(glm::translate(glm::mat4(1.0f), glm::vec3(-50.0f, 10.25f, 0.0f)), Geometry::createCubeGeometry(1.f, 50.5f, length), wallMat);
		Geometry wallFront = Geometry(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.25f, 50.0f)), Geometry::createCubeGeometry(width, 50.5f, 1.f), wallMat);
		Geometry wallBack = Geometry(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.25f, -50.0f)), Geometry::createCubeGeometry(width, 50.5f, 1.f), wallMat);
		Geometry decke = Geometry(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 35.5f, 0.0f)), glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f)), Geometry::createCubeGeometry(width, 1.f, length), wallMat);
		Geometry decke2 = Geometry(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 30.5f, -20.0f)), glm::radians(18.0f), glm::vec3(0.0f, -1.0f, -1.0f)), Geometry::createCubeGeometry(width, 1.f, length), wallMat);
		Geometry decke3 = Geometry(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 25.5f, 20.0f)), glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f)), Geometry::createCubeGeometry(width, 1.f, length), wallMat);


		Geometry wall01 = Geometry(glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 3.25f, 0.0f)), Geometry::createCubeGeometry(1.f, 5.f, 20), wallMat);
		pWorld->addCubeToPWorld(wall01, glm::vec3(1.f, 5.f, 20) * 0.5f);


		pWorld->addCubeToPWorld(ground, glm::vec3(width, 1.f, length) * 0.5f);
		pWorld->addCubeToPWorld(wallRight, glm::vec3(1.f, 50.5f, length) * 0.5f);
		pWorld->addCubeToPWorld(wallLeft, glm::vec3(1.f, 50.5f, length) * 0.5f);
		pWorld->addCubeToPWorld(wallBack, glm::vec3(width, 50.5f, 1.f) * 0.5f);
		pWorld->addCubeToPWorld(wallFront, glm::vec3(width, 50.5f, 1.f) * 0.5f);
		pWorld->addCubeToPWorld(decke, glm::vec3(width, 1.f, length) * 0.5f);
		pWorld->addCubeToPWorld(decke2, glm::vec3(width, 1.f, length) * 0.5f);
		pWorld->addCubeToPWorld(decke3, glm::vec3(width, 1.f, length) * 0.5f);




		//set players body and hand
		//Geometry body = Geometry(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 3.0f)), Geometry::createCylinderGeometry(32, 3.5f, 1.0f), woodTextureMaterial);
		Geometry hitboxPlayer = Geometry(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 6.0f, 35.0f)), Geometry::createCubeGeometry(1.0f, 3.50f, 1.0f), playerMat );
		pWorld->addPlayerToPWorld(player, glm::vec3(1.0f, 3.5f, 1.0f) * 0.5f);

		//THE HELLISH DODGEBALL IS COMING FOR YOU
		Geometry THEHELLISHDODGEBALL = Geometry(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 6.5f, -10.0f)), Geometry::createSphereGeometry(64, 32, 0.5f), ballMat);
		pWorld->addSphereToPWorld(THEHELLISHDODGEBALL, 0.5f, false);
		
		//TEXT
		Text score = Text("Score: ", glm::vec2(50.0f, 100.0f), 1.f, glm::vec3(1.0f, 0.2f, 0.2f), _characters,*hudShader.get());
		Text endOfGame = Text("THE HELLISH DODGEBALL GOT YOU", glm::vec2(window_width/2.0f - 580, window_height-300.0f), 2.f, glm::vec3(1, 0,0), _characters, *hudShader.get());
		Text endOfGameTrap = Text("YOU HAVE FALLEN INTO A TRAP!", glm::vec2(window_width/2.0f - 530, window_height-300.0f), 2.f, glm::vec3(1, 0,0), _characters, *hudShader.get());
		Text endScore = Text("Your Score: ", glm::vec2(50.0f, 100.0f), 1.f, glm::vec3(1.0f, 0.2f, 0.2f), _characters, *hudShader.get());
		Text highScore = Text("Highscore: ", glm::vec2(850.0f, 1000.0f), 1.f, glm::vec3(1, 0.2f, 0.5f), _characters, *hudShader.get());
		Text deathHighScore = Text("Hig-.hscore: ", glm::vec2(50.0f, 200.0f), 1.f, glm::vec3(1, 0.2f, 0.5f), _characters, *hudShader.get());
		Text enterToRestart = Text("Press \"enter\" to restart", glm::vec2(window_width - 500.f, 100.0f), 1.f, glm::vec3(1.0f, 0.2f, 0.2f), _characters, *hudShader.get());
		
		int scoreCounter;

		// Initialize lights
		
		PointLight PointMid(glm::vec3(1.f), glm::vec3(0.0f, 8.f, 0.0f), glm::vec3(0.05f));

		PointLight ball = PointLight(glm::vec3(1, 0.2f, 0), pWorld->getBallPosition(), glm::vec3(.45f, 0.55f, 0.0f));
		bulbs.push_back(&THEHELLISHDODGEBALL.getModelMatrix());
		pointLights.push_back(&ball);

		double mouse_x, mouse_y;
		
		glm::mat4 projMatrix = glm::perspective(glm::radians(fov), (float)window_width / (float)window_height, nearZ, farZ);
		player.getCamera()->setProjectionMatrix(projMatrix);

	

		// ---------------------------------------
		//  Set up Buffers for Post-Processing
		//  //the following buffer code was taken
		//  from https://learnopengl.com/Advanced-Lighting/Bloom
		// ---------------------------------------


		// configure (floating point) framebuffers
		// -----------------------------------------------------------------------------------------------------------------------------------------------------------------
		unsigned int hdrFBO;
		glGenFramebuffers(1, &hdrFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		// create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
		glGenTextures(2, colorBuffers);
		for (unsigned int i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window_width, window_height, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// attach texture to framebuffer
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
		}
		// create and attach depth buffer (renderbuffer)
		unsigned int rboDepth;
		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window_width, window_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
		// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
		// finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// ping-pong-framebuffer for blurring
		glGenFramebuffers(2, pingpongFBO);
		glGenTextures(2, pingpongColorbuffers);
		for (unsigned int i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
			glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window_width, window_height, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
			// also check if framebuffers are complete (no need for depth buffer)
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "Framebuffer not complete!" << std::endl;
		}
		//cookTexturedShader->use();
		//cookTexturedShader->setUniform("diffuseTexture", 0);
		blurShader->use();
		blurShader->setUniform("image", 0);
		bloomShader->use();
		bloomShader->setUniform("scene",0);
		bloomShader->setUniform("bloomBlur", 1);



		bool bloom = true;
		float exposure = 1.f;
		// -----------------------------------------------------------------------------------------------------------------------------------------------------------------

		string dashString = "";
		Timer dashTimer = Timer();


		// ---------------------------------------
		// Render Loop
		// ---------------------------------------

		while (!glfwWindowShouldClose(window)) {

			// Clear backbuffer
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			// Poll events
			glfwPollEvents();

			// Update camera
			glfwGetCursorPos(window, &mouse_x, &mouse_y);
			processKeyInput(window);


			// Buffers for post processing
			glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			Camera* cam = player.getCamera();
			glm::vec3 ballPosition = pWorld->getBallPosition();
			pointLights[pointLights.size() - 1] = &PointLight(glm::vec3(1, 0.2f, 0), ballPosition, glm::vec3(.45f, 0.55f, 0.0f));
			bulbs[bulbs.size() - 1] = &THEHELLISHDODGEBALL.getModelMatrix();
			
			//Update our Dynamic Actors
			pWorld->updatePlayer(PNOMOVEMENT, deltaTime);
			//pWorld->updateBall();

			
			//set the uniforms for the animation shader
			for (int i = 0; i < pointLights.size(); i++) {
				setPerFrameUniforms(animationShader.get(), *cam, cam->getProjectionMatrix(), *pointLights[i], i);
			}

			//set the uniforms for the texture shader
			for (int i = 0; i < pointLights.size(); i++) {
				setPerFrameUniforms(textureShader.get(), *cam, cam->getProjectionMatrix(), *pointLights[i], i);
			}

			//set the uniforms for the physically based shader
			for (int i = 0; i < pointLights.size(); i++) {
				setPerFrameUniforms(cookTexturedShader.get(), *cam, cam->getProjectionMatrix(), *pointLights[i], i);
			}

			

			// ---------------------------------------
			// DRAW STUFF
			// ---------------------------------------

			//models
			drawModelVector(&torch,torches);
			armModel.Draw(armModel.getModel());
			
		
			//everything registered in the physicsworld
			pWorld->draw();
			
			//Traps + lava
			//drawTrapsOrLava(traps);
			//drawTrapsOrLava(lava,false);

			// "lightbulbs" for the torches
			for (int i = 0; i < bulbs.size(); i++) {
				setPerFrameUniforms(lightMakerShader.get(), *cam, cam->getProjectionMatrix(), *pointLights[i], i);
				
				if (i == bulbs.size() - 1)
				{
					bulb.setModelMatrix(glm::scale( *bulbs[i], glm::vec3(1.f, 1.2f, 1.f)));
					//bulb.draw(lightMakerShader.get());
					break;
				}
				bulb.setModelMatrix(*bulbs[i]);
				bulb.draw(lightMakerShader.get());
				
				
			}


		
			
			
			

			//Text
			scoreCounter = pWorld->getScoreCounter();

			//End of game Condition
			if (pWorld->isPlayerDead() || pWorld->isPlayerHit()) 
			{
					Timer endTimer = Timer();
					isDead = true;
					resetGame = false;
					

					while (!glfwWindowShouldClose(window) && !resetGame) {

						glfwPollEvents();
						processKeyInput(window);

						
						if (pWorld->isPlayerHit()) {
							endOfGame.setText("*THE HELLISH DODGEBALL GOT YOU!*");
							endOfGame.drawText();
						}
						if (pWorld->isPlayerDead()) {
							endOfGameTrap.setText("*YOU HAVE FALLEN INTO A TRAP!*");
							endOfGameTrap.drawText();
						}

						
						endScore.setText("Your Score: " + std::to_string(scoreCounter * 10));

						if (highscore <= scoreCounter) {
							deathHighScore.setText("Your New Highscore: " + std::to_string(scoreCounter * 10));
							highscore = scoreCounter;
						}
						else {
							deathHighScore.setText("Highscore: " + std::to_string(highscore * 10));
						}

						deathHighScore.drawText();
						endScore.drawText();
						enterToRestart.drawText();
						dashCooldownDraw(*hudShader.get());



						postProcessing(blurShader, bloomShader, bloom, exposure);
						glfwSwapBuffers(window);
					}
			}

			//HUD
			score.setText("Score: " + std::to_string(scoreCounter * 10));
			score.drawText();
			highScore.drawText();
			dashCooldownDraw(*hudShader.get());
			//dashCooldown.drawText(true);
			
			//apply bloom
			postProcessing(blurShader, bloomShader, bloom, exposure);

			//time logic
			float currentFrame = static_cast<float>(glfwGetTime());
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			pScene->simulate(deltaTime);
			pScene->fetchResults(true);

			// Swap buffers
			glfwSwapBuffers(window);

		}

		//-particles.DestroyParticleSystem();
	}


	/* --------------------------------------------- */
	// Destroy framework
	/* --------------------------------------------- */

	destroyFramework();


	/* --------------------------------------------- */
	// Destroy context and exit
	/* --------------------------------------------- */

	glfwTerminate();

	return EXIT_SUCCESS;
}


//set Shader Uniforms if scene is lit by a directional light and a pointlight
void setPerFrameUniforms(Shader* shader, Camera& camera, glm::mat4 projMatrix, DirectionalLight& dirL, PointLight& pointL)
{
	shader->use();
	shader->setUniform("viewMatrix", player.getCamera()->GetViewMatrix());
	shader->setUniform("projMatrix", projMatrix);
	shader->setUniform("camera_world", player.getCamera()->getPosition());

	shader->setUniform("dirL.color", dirL.color);
	shader->setUniform("dirL.direction", dirL.direction);
	shader->setUniform("pointL.color", pointL.color);
	shader->setUniform("pointL.position", pointL.position);
	shader->setUniform("pointL.attenuation", pointL.attenuation);
}
//set shader Uniforms if scene is lit by directional light
void setPerFrameUniforms(Shader* shader, Camera& camera, glm::mat4 projMatrix, DirectionalLight& dirL, glm::vec3 color)
{
	shader->use();
	shader->setUniform("viewMatrix", player.getCamera()->GetViewMatrix());
	shader->setUniform("projMatrix", projMatrix);
	shader->setUniform("camera_world", player.getCamera()->getPosition());

	shader->setUniform("lightColor", dirL.color);
	shader->setUniform("lightDirection", dirL.direction);
	shader->setUniform("objectColor", color);

}

//set shader Uniforms if scene is lit by a poinlight
void setPerFrameUniforms(Shader* shader, Camera& camera, glm::mat4 projMatrix, PointLight& pointL)
{
	shader->use();
	shader->setUniform("viewMatrix", player.getCamera()->GetViewMatrix());
	shader->setUniform("projMatrix", projMatrix);
	shader->setUniform("camera_world", player.getCamera()->getPosition());

	
	shader->setUniform("pointL.color", pointL.color);
	shader->setUniform("pointL.position", pointL.position);
	shader->setUniform("pointL.attenuation", pointL.attenuation);
}

//set shader Uniforms if scene is lit by multiple pointlights
void setPerFrameUniforms(Shader* shader, Camera& camera, glm::mat4 projMatrix, PointLight& pointL, int lightID)
{
	shader->use();
	shader->setUniform("viewMatrix", player.getCamera()->GetViewMatrix());
	shader->setUniform("projMatrix", projMatrix);
	shader->setUniform("camera_world", player.getCamera()->getPosition());


	shader->setUniform("pointLights[" + std::to_string(lightID) + "].color", pointL.color);
	shader->setUniform("pointLights[" + std::to_string(lightID) + "].position", pointL.position);
	shader->setUniform("pointLights[" + std::to_string(lightID) + "].attenuation", pointL.attenuation);	
}

//draw multiple geometry objects stored in a vector
void drawGeometryVector(std::vector<Geometry*> x) 
{

	Geometry* tmp;
	std::vector<Geometry*>::iterator it;
	for (it = x.begin(); it != x.end(); it++) {
		tmp = *it;
		tmp->draw();
	}
}

//draw traps or lava
void drawTrapsOrLava(std::vector<Geometry*> x, boolean isTrap)
{

	Geometry* tmp;
	std::vector<Geometry*>::iterator it;
	Camera* cam = player.getCamera();
	for (it = x.begin(); it != x.end(); it++) {
		tmp = *it;
		if (isTrap) {
			tmp->draw();
		}else {
			tmp->draw(static_cast<float>(glfwGetTime()));
		}
	}

}

//draw multiple models stored in a vector
void drawModelVector(Model* model, std::vector<glm::mat4*> x) 
{

	glm::mat4* tmp;
	std::vector<glm::mat4*>::iterator it;
	for (it = x.begin(); it != x.end(); it++) {
		tmp = *it;
		model->Draw(*tmp);
	}
}

//draw a particle system
void drawPSVector(std::vector<ParticleSystem*> x,float deltaTime, float newparticles) 
{

	ParticleSystem* tmp;
	std::vector<ParticleSystem*>::iterator it;
	for (it = x.begin(); it != x.end(); it++) {

		tmp = *it;
		tmp->Update(deltaTime, newparticles);
		tmp->Draw();
	}
}

//draws the dash cooldown on the HUD
void dashCooldownDraw(Shader& shader) {
	
	bool hasDashed = pWorld->playerHasDashed();
	static string dashString = "";
	static Timer dashTimer = Timer();
	static Text dashCooldown = Text("IIIIIIIIII", glm::vec2(window_width-300, 100.0f), 2.f, glm::vec3(0.2f, 1, 0.2f), _charactersForCooldown, shader);

	if (isDead) {
		dashCooldown.setText("IIIIIIIIII");
		dashCooldown.setColor(glm::vec3(0.1f, 1.0f, 0.1f));
		dashString = "";
		dashTimer.Reset();
		return;
	}

	if (hasDashed) {

		if (dashTimer.Duration() >= 0.25f) {
			dashCooldown.setText(dashString.append("I"));
			dashCooldown.setColor(glm::vec3(1.f, 0.1f, 0.1f));
			dashTimer.Reset();
		}

	}
	else {

		dashCooldown.setText("IIIIIIIIII");
		dashCooldown.setColor(glm::vec3(0.1f, 1.0f, 0.1f));
		dashString = "";
	}

	dashCooldown.drawText(true);
}

//create hitbox for trap so the ball does not get stuck under the floor
void createTrapHitbox(glm::vec3 position, float size, std::shared_ptr<Material> wallMat)
{
	Geometry frontWall = Geometry(glm::translate(glm::mat4(1.0f), glm::vec3(position.x - (size/2), 0,position.z)), Geometry::createCubeGeometry(0.1f, 4.f, size), wallMat);
	Geometry backWall = Geometry(glm::translate(glm::mat4(1.0f), glm::vec3(position.x + (size/2), 0,position.z)), Geometry::createCubeGeometry(0.1f, 4.f, size), wallMat);
	Geometry rightWall = Geometry(glm::translate(glm::mat4(1.0f), glm::vec3(position.x, 0,position.z + (size/2))), Geometry::createCubeGeometry(size, 4.f, 0.1f), wallMat);
	Geometry leftWall = Geometry(glm::translate(glm::mat4(1.0f), glm::vec3(position.x, 0,position.z - (size/2))), Geometry::createCubeGeometry(size, 4.f, 0.1f), wallMat);
	pWorld->addCubeToPWorld(frontWall, glm::vec3(0.1f, 4.f, size) * 0.5f, true, true);
	pWorld->addCubeToPWorld(backWall, glm::vec3(0.1f, 4.f, size) * 0.5f, true, true);
	pWorld->addCubeToPWorld(rightWall, glm::vec3(size, 4.f, 0.1f) * 0.5f, true, true);
	pWorld->addCubeToPWorld(leftWall, glm::vec3(size, 4.f, 0.1f) * 0.5f, true, true);

}

//creates the torches on the wall
std::vector<glm::mat4*> createTorches(Model torch, std::shared_ptr<Material> mat)
{
	
	std::vector<glm::mat4*> torches;
	Geometry torchHitBox = Geometry(glm::scale(glm::mat4(1.0f), glm::vec3(10)), Geometry::createCubeGeometry(64.f, 32.f, 8.5f) , mat);
	for (int i = -1; i <= 0; i++) {
		
		//torches at front of the room
		glm::mat4* tmpTorch = new glm::mat4(glm::scale(glm::rotate(glm::translate(torch.getModel(), glm::vec3(i * 20.f, 7.f, -49.5f)), glm::radians(0.f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(0.05f, 0.05f, 0.05f)));
		torches.push_back(tmpTorch);

		torchHitBox = Geometry(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(i * 20.f, 7.5f, -48.0f)), glm::vec3(1)), Geometry::createCubeGeometry(64.f, 32.f, 8.5f), mat);
		pWorld->addCubeToPWorld(torchHitBox, glm::vec3(1.f, 4.f, 1.f) * 0.5f, true, true);
		
		//torches at back of the room
		glm::mat4* tmpTorch1 = new glm::mat4(glm::scale(glm::rotate(glm::translate(torch.getModel(), glm::vec3(-i * 20.f, 7.f, 49.5f)), glm::radians(180.f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(0.05f, 0.05f, 0.05f)));
		torches.push_back(tmpTorch1);
		
		torchHitBox = Geometry(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-i * 20.f, 7.5f, 48.0f)), glm::vec3(1)), Geometry::createCubeGeometry(64.f, 32.f, 8.5f), mat);
		pWorld->addCubeToPWorld(torchHitBox, glm::vec3(1.f, 4.f, 1.f) * 0.5f,true,true);	

		//torches at right of the room
		glm::mat4* tmpTorch2 = new glm::mat4(glm::scale(glm::rotate(glm::translate(torch.getModel(), glm::vec3(49.5f, 7.f, i * 20.0f)), glm::radians(-90.f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(0.05f, 0.05f, 0.05f)));
		torches.push_back(tmpTorch2);

		torchHitBox = Geometry(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(48.0f, 7.5f, i * 20.0f)), glm::vec3(1)), Geometry::createCubeGeometry(64.f, 32.f, 8.5f), mat);
		pWorld->addCubeToPWorld(torchHitBox, glm::vec3(1.f, 4.f, 1.f) * 0.5f, true, true);
		
		//torches at left of the room
		glm::mat4* tmpTorch3 = new glm::mat4(glm::scale(glm::rotate(glm::translate(torch.getModel(), glm::vec3(-49.5f, 7.f, -i * 20.0f)), glm::radians(90.f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(0.05f, 0.05f, 0.05f)));
		torches.push_back(tmpTorch3);

		torchHitBox = Geometry(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-48.0f, 7.5f, -i * 20.0f)), glm::vec3(1)), Geometry::createCubeGeometry(64.f, 32.f, 8.5f), mat);
		pWorld->addCubeToPWorld(torchHitBox, glm::vec3(1.f, 4.f, 1.f) * 0.5f, true, true);
		
	}

	return torches;
}

//creates the "lightbulbs" for the torches to add a bloom
std::vector<glm::mat4*> createBulbs()
{

	std::vector<glm::mat4*> bulbs;

	for (int i = -1; i <= 0; i++) {

		//torches at front of the room
		glm::mat4* bulbF = new glm::mat4(glm::translate(glm::mat4(1.f), glm::vec3(i * 20.f, 8.9f, -48.25f)));
		bulbs.push_back(bulbF);

		//torches at back of the room
		glm::mat4* bulbB = new glm::mat4(glm::translate(glm::mat4(1.f), glm::vec3(-i * 20.f, 8.9f, 48.25f)));
		bulbs.push_back(bulbB);

		//torches at right of the room
		glm::mat4* bulbR = new glm::mat4(glm::translate(glm::mat4(1.f), glm::vec3(48.25f, 8.9f, i * 20.0f)));
		bulbs.push_back(bulbR);

		//torches at left of the room
		glm::mat4* bulbL = new glm::mat4(glm::translate(glm::mat4(1.f), glm::vec3(-48.25f, 8.9f, -i * 20.0f)));
		bulbs.push_back(bulbL);


	}

	return bulbs;
}

//create all the lights for the torches
std::vector<PointLight*> createLights(glm::vec3 flamecolor)
{
	std::vector<PointLight*> lights;

	for (int i = -1; i <= 0; i++) {
		
		//lights at the front of the room 
		PointLight* tmpPoint = new PointLight(flamecolor, glm::vec3(i*20.f, 9.f, -46.5f), glm::vec3(0.f,0.f, 0.005f));
		lights.push_back(tmpPoint);

		//lights at the back of the room 
		PointLight* tmpPoint1 = new PointLight(flamecolor, glm::vec3(-i*20.f, 9.f, 46.5f), glm::vec3(0.0f, 0.f, 0.006f));
		lights.push_back(tmpPoint1);

		//lights at the right of the room 
		PointLight* tmpPoint2 = new PointLight(flamecolor, glm::vec3(46.5f, 9.f, i*20.f), glm::vec3(0.f, 0.f, 0.007f));
		lights.push_back(tmpPoint2);

		//lights at the left of the room 
		PointLight* tmpPoint3 = new PointLight(flamecolor, glm::vec3(-46.5f, 9.f, -i*20.f), glm::vec3(0.f,0.f,0.008f));
		lights.push_back(tmpPoint3);
	}

	return lights;
}

//create a quad that fills the screen for the post processing
void createQuad()
{
	if (qVAO == 0)
	{
		float qVertices[] = {
			// (x,	   y,    z) & (u,    v)
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
	    //create VAO
        glGenVertexArrays(1, &qVAO);
        glGenBuffers(1, &qVBO);
		//create VBO
        glBindVertexArray(qVAO);
        glBindBuffer(GL_ARRAY_BUFFER, qVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(qVertices), &qVertices, GL_STATIC_DRAW);

		//bind vertices to location 0
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		//bind uvs to location 2
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
	//draw quad
    glBindVertexArray(qVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

//apply bloom and glow
void postProcessing(shared_ptr<Shader> blurShader, shared_ptr<Shader> bloomShader, bool bloom, float exposure)
{
	// 2. blur bright fragments with two-pass Gaussian Blur 
	// --------------------------------------------------
	bool horizontal = true, first_iteration = true;
	unsigned int amount = 16;
	blurShader->use();
	for (unsigned int i = 0; i < amount; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
		blurShader->setUniform("horizontal", horizontal);
		glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
		createQuad();
		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
	// --------------------------------------------------------------------------------------------------------------------------
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	bloomShader->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);

	bloomShader->setUniform("bloom", bloom);
	bloomShader->setUniform("exposure", exposure);
	createQuad();
}

//processes all key inputs
void processKeyInput(GLFWwindow* window) 
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
	{

		if (isDead) 
		{
			pWorld->resetGame();
			resetGame = true;
			isDead = false;
		}
	}


	if (!isDead) {

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			pWorld->updatePlayer(PFORWARD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			pWorld->updatePlayer(PBACKWARD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			pWorld->updatePlayer(PLEFT, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			pWorld->updatePlayer(PRIGHT, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			pWorld->updatePlayer(PJUMP, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			pWorld->updatePlayer(PDASH, deltaTime);
		}
		
	}
}

//callback for mouse movement
void mouse_callback(GLFWwindow* window, double x, double y)
{
	float xpos = static_cast<float>(x);
	float ypos = static_cast<float>(y);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	player.ProcessMouseMovement(xoffset, yoffset);
}

//callback for mouse scroll
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	player.ProcessMouseScroll(static_cast<float>(yoffset));
}

//callback for mouse buttons
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{	
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pWorld->Animate(player); 
		
	} else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		_dragging = false;
	} else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		_strafing = true;
	} else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		_strafing = false;
	}
}

//key callback for wireframe and culling
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// F1 - Wireframe
	// F2 - Culling
	// Esc - Exit

	if (action != GLFW_RELEASE) return;

	switch (key)
	{
		case GLFW_KEY_LEFT_SHIFT:
			
			break;
		case GLFW_KEY_2:
			if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}

			break;
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, true);
			break;
		case GLFW_KEY_F1:
			_wireframe = !_wireframe;
			glPolygonMode(GL_FRONT_AND_BACK, _wireframe ? GL_LINE : GL_FILL);
			break;
		case GLFW_KEY_F2:
			_culling = !_culling;
			if (_culling) glEnable(GL_CULL_FACE);
			else glDisable(GL_CULL_FACE);
			break;
	}
}

//DEBUG 
static void APIENTRY DebugCallbackDefault(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam) 
{
	if (id == 131185 || id == 131218 || id==131185) return; // ignore performance warnings from nvidia
	std::string error = FormatDebugOutput(source, type, id, severity, message);
	std::cout << error << std::endl;
}
//DEBUG
static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg) 
{
	std::stringstream stringStream;
	std::string sourceString;
	std::string typeString;
	std::string severityString;

	// The AMD variant of this extension provides a less detailed classification of the error,
	// which is why some arguments might be "Unknown".
	switch (source) {
	case GL_DEBUG_CATEGORY_API_ERROR_AMD:
	case GL_DEBUG_SOURCE_API: {
		sourceString = "API";
		break;
	}
	case GL_DEBUG_CATEGORY_APPLICATION_AMD:
	case GL_DEBUG_SOURCE_APPLICATION: {
		sourceString = "Application";
		break;
	}
	case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
		sourceString = "Window System";
		break;
	}
	case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
	case GL_DEBUG_SOURCE_SHADER_COMPILER: {
		sourceString = "Shader Compiler";
		break;
	}
	case GL_DEBUG_SOURCE_THIRD_PARTY: {
		sourceString = "Third Party";
		break;
	}
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_SOURCE_OTHER: {
		sourceString = "Other";
		break;
	}
	default: {
		sourceString = "Unknown";
		break;
	}
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR: {
		typeString = "Error";
		break;
	}
	case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
		typeString = "Deprecated Behavior";
		break;
	}
	case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
		typeString = "Undefined Behavior";
		break;
	}
	case GL_DEBUG_TYPE_PORTABILITY_ARB: {
		typeString = "Portability";
		break;
	}
	case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
	case GL_DEBUG_TYPE_PERFORMANCE: {
		typeString = "Performance";
		break;
	}
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_TYPE_OTHER: {
		typeString = "Other";
		break;
	}
	default: {
		typeString = "Unknown";
		break;
	}
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: {
		severityString = "High";
		break;
	}
	case GL_DEBUG_SEVERITY_MEDIUM: {
		severityString = "Medium";
		break;
	}
	case GL_DEBUG_SEVERITY_LOW: {
		severityString = "Low";
		break;
	}
	default: {
		severityString = "Unknown";
		break;
	}
	}

	stringStream << "OpenGL Error: " << msg;
	stringStream << " [Source = " << sourceString;
	stringStream << ", Type = " << typeString;
	stringStream << ", Severity = " << severityString;
	stringStream << ", ID = " << id << "]";

	return stringStream.str();
}