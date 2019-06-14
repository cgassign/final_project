#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <cmath>
#include <algorithm>
#include <vector>
#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw_gl3.h"
#include "model.h"
#include "camera.h"
#include "shader.h"
#include <iostream>

#define bezier_point_num 1000
using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadCubemap(vector<std::string> faces);

// 设置窗口大小
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;

// 摄像机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// 计时
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	glfwInit();
	// 初始化GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// 创建窗口对象
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGLTest", NULL, NULL);
	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// 告诉OpenGL渲染窗口的尺寸大小，用glViewport函数设置窗口维度
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	// 让GFLW捕捉鼠标
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// 在调用OpenGL函数前初始化GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	// 开启深度测试
	glEnable(GL_DEPTH_TEST);
	// 构造shader
	Shader ourShader("model_loading.vs", "model_loading.fs");
	Shader skyboxShader("skybox.vs", "skybox.fs");
	// 加载模型
	Model tree("resource/tree/tree.obj");
	Model player("resource/nanosuit/nanosuit.obj");
	Model house1("resource/Building Apartment/Mesh/Building_Apartment_10.fbx");
	Model house2("resource/Building Apartment/Mesh/Building_Apartment_13.fbx");
	//Model ourModel("resource/TT_demo_police.FBX");

	// 地板
	float planeVertices[] = {
		// positions            // normals         // texcoords
		25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};
	// plane VBO
	unsigned int planeVBO;
	unsigned int planeVAO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
	// 导入地板的贴图
	unsigned int woodTexture;// = loadTexture(FileSystem::getPath("resources/wood.png").c_str());
	glGenTextures(1, &woodTexture);
	int width, height, nrComponents;
	unsigned char *data = stbi_load("resource/Ground/grass.tga", &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, woodTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path " << std::endl;
		stbi_image_free(data);
	}

	// 天空盒顶点数组
	float skyboxVertices[] = {
		// positions          
		-25.0f,  25.0f, -25.0f,
		-25.0f, -25.0f, -25.0f,
		25.0f, -25.0f, -25.0f,
		25.0f, -25.0f, -25.0f,
		25.0f,  25.0f, -25.0f,
		-25.0f,  25.0f, -25.0f,

		-25.0f, -25.0f,  25.0f,
		-25.0f, -25.0f, -25.0f,
		-25.0f,  25.0f, -25.0f,
		-25.0f,  25.0f, -25.0f,
		-25.0f,  25.0f,  25.0f,
		-25.0f, -25.0f,  25.0f,

		25.0f, -25.0f, -25.0f,
		25.0f, -25.0f,  25.0f,
		25.0f,  25.0f,  25.0f,
		25.0f,  25.0f,  25.0f,
		25.0f,  25.0f, -25.0f,
		25.0f, -25.0f, -25.0f,

		-25.0f, -25.0f,  25.0f,
		-25.0f,  25.0f,  25.0f,
		25.0f,  25.0f,  25.0f,
		25.0f,  25.0f,  25.0f,
		25.0f, -25.0f,  25.0f,
		-25.0f, -25.0f,  25.0f,

		25.0f,  25.0f,  25.0f,
		-25.0f,  25.0f, -25.0f,
		-25.0f,  25.0f, 25.0f,
		25.0f,  25.0f, -25.0f,
		-25.0f,  25.0f, -25.0f,
		25.0f,  25.0f,  25.0f,
	

		-25.0f, -25.0f, -25.0f,
		-25.0f, -25.0f,  25.0f,
		25.0f, -25.0f, -25.0f,
		25.0f, -25.0f, -25.0f,
		-25.0f, -25.0f,  25.0f,
		25.0f, -25.0f,  25.0f
	};

	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// 天空盒贴图
	vector<std::string> faces
	{
		"resource/skybox/left.tga",
		"resource/skybox/right.tga",
		"resource/skybox/top.tga",
		"resource/skybox/bottom.tga",
		"resource/skybox/back.tga",
		"resource/skybox/front.tga"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	//渲染循环
	while (!glfwWindowShouldClose(window))
	{
		// 获取每帧时间
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		// 输入
		processInput(window);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ourShader.use();

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		// 渲染模型
		glm::mat4 plane_model = glm::mat4(1.0f);
		// 渲染平面
		plane_model = glm::translate(plane_model, glm::vec3(0.0f, -1.5f, 0.0f));
		ourShader.setMat4("model", plane_model);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// 调整树的位置
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-1.0f, -1.5f, -2.0f));
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));	
		ourShader.setMat4("model", model);
		tree.Draw(ourShader);
		model = glm::translate(model, glm::vec3(40.0f, 0.0f, 0.0f));
		ourShader.setMat4("model", model);
		tree.Draw(ourShader);

		// 调整房屋的位置
		glm::mat4 house_model = glm::mat4(1.0f);
		house_model = glm::translate(house_model, glm::vec3(5.0f, -1.5f, -8.0f));
		house_model = glm::scale(house_model, glm::vec3(0.0025f, 0.0025f, 0.0025f));
		ourShader.setMat4("model", house_model);
		house1.Draw(ourShader);

		glm::mat4 house_model2 = glm::mat4(1.0f);
		house_model2 = glm::translate(house_model2, glm::vec3(-3.0f, -1.5f, -5.0f));
		house_model2 = glm::scale(house_model2, glm::vec3(0.0025f, 0.0025f, 0.0025f));
		ourShader.setMat4("model", house_model2);
		house2.Draw(ourShader);

		glm::mat4 player_model = glm::mat4(1.0f);
		// 调整人物的位置
		player_model = glm::translate(player_model, glm::vec3(0.0f, -1.5f, 0.0f));
		player_model = glm::scale(player_model, glm::vec3(0.05f, 0.05f, 0.05f));
		player_model = glm::rotate(player_model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ourShader.setMat4("model", player_model);
		player.Draw(ourShader);

		// 画天空盒
		// 更改深度函数，以便深度测试在值等于深度缓冲区的内容时通过
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		// 从视图矩阵中删除平移
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		// 天空盒
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		// 深度设置回默认值
		glDepthFunc(GL_LESS);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();

	return 0;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// 当窗口大小变化，调用这个函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// 确保视点和窗口大小匹配
	glViewport(0, 0, width, height);
}

// 鼠标移动的回调函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; 

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// 鼠标滚轮的回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// 加载天空盒代码
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}