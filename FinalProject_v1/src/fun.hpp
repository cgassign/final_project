#include "shader.h"
#include "camera.h"
#include "model.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad\glad.h>
#include <GLFW\glfw3.h>

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

// 定义字符
struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint Advance;    // Horizontal offset to advance to next glyph
};
std::map<GLchar, Character> Characters;

struct modelCollision {
	glm::vec3 center;
	float lengthLeft;
	float lengthRight;
	float widthUp;
	float widthBack;
	modelCollision(glm::vec3 centerIn, float lengthLeftIn, float lengthRightIn, float widthUpIn, float widthBackIn) {
		center = centerIn;
		lengthLeft = lengthLeftIn;
		lengthRight = lengthRightIn;
		widthUp = widthUpIn;
		widthBack = widthBackIn;
	}
};

void setSomething();
void playerMovement(int key);
bool checkCollision();
void changePlayerFaceTo();
glm::vec3 getCameraFollowPlayer();

void RenderText(Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void renderScene(const Shader &shader, vector<Model> &allModels);
void renderFloor();
void renderLight();

unsigned int loadCubemap(vector<std::string> faces);
void renderSkybox();

// settings
unsigned int SCR_WIDTH = 1200;
unsigned int SCR_HEIGHT = 1200;

// camera
//Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
Camera *camera;

Camera playerCamera(glm::vec3(0.0f, 0.0f, 10.0f));
Camera viewCamera(glm::vec3(0.0f, 0.0f, 10.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// meshes
unsigned int planeVAO = 0;
unsigned int planeVBO = 0;

unsigned int lightVAO = 0;
unsigned int lightVBO = 0;

unsigned int skyboxVAO = 0;
unsigned int skyboxVBO = 0;

unsigned int textVAO = 0;
unsigned int textVBO = 0;

unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

glm::vec3 lightPos(3.0f, 9.0f, -6.0f);
glm::vec3 playerPos(0.0f, -0.5f, 0.0f);

vector<modelCollision> modelVec;

glm::vec3 treePos1(-1.0f, -0.5f, -2.0f);
glm::vec3 treePos2(4.0f, -0.5f, 0.0f);
glm::vec3 treePos3(-1.5f, -0.5f, -3.5f);
glm::vec3 treePos4(3.5f, -0.5f, -3.5f);

glm::vec3 housePos1(5.0f, -0.5f, -8.0f);
glm::vec3 housePos2(-1.0f, -0.5f, -10.0f);
glm::vec3 housePos3(-3.5f, -0.5f, -5.5f);
glm::vec3 housePos4(1.0f, -0.5f, -6.0f);

glm::mat4 player_model = glm::mat4(1.0f);
float rotateAngle = 0.0f;
float pi_2 = glm::acos(-1.0f);
bool followPlayer = false;
bool isGamma = false;

glm::vec3 lastRotateFront = glm::vec3(0.0f, 0.0f, -1.0f);
bool isFirstFrame = true;

void setSomething() {
	camera = &viewCamera;

	modelCollision treeModel1(treePos1, 0.9f, 0.75f, 0.0f, 1.3f);
	modelCollision treeModel2(treePos2, 0.9f, 0.75f, 0.0f, 1.3f);
	modelCollision treeModel3(treePos3, 0.9f, 0.75f, 0.0f, 1.3f);
	modelCollision treeModel4(treePos4, 0.9f, 0.75f, 0.0f, 1.3f);

	modelCollision houseModel1(housePos1, 1.6f, 1.6f, 1.0f, 1.0f);
	modelCollision houseModel2(housePos2, 1.6f, 1.6f, 1.0f, 1.0f);
	modelCollision houseModel3(housePos3, 1.0f, 1.0f, 1.0f, 1.0f);
	modelCollision houseModel4(housePos4, 1.0f, 1.0f, 1.0f, 1.0f);

	modelVec.push_back(treeModel1);
	modelVec.push_back(treeModel2);
	modelVec.push_back(treeModel3);
	modelVec.push_back(treeModel4);
	modelVec.push_back(houseModel1);
	modelVec.push_back(houseModel2);
	modelVec.push_back(houseModel3);
	modelVec.push_back(houseModel4);
}

bool checkCollision() {
	for (int i = 0; i < modelVec.size(); i++) {
		if ((playerPos[0] < modelVec[i].center[0] + modelVec[i].lengthRight && playerPos[0] > modelVec[i].center[0] - modelVec[i].lengthLeft) &&
			(playerPos[2] < modelVec[i].center[2] + modelVec[i].widthUp && playerPos[2] > modelVec[i].center[2] - modelVec[i].widthBack))
		{

			return true;
		}
	}
	return false;
}

void RenderText(Shader &shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
	// Activate corresponding render state	
	shader.use();
	//glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
	
	shader.setVec3("textColor", color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(textVAO);

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
		{ xpos,     ypos,       0.0, 1.0 },
		{ xpos + w, ypos,       1.0, 1.0 },

		{ xpos,     ypos + h,   0.0, 0.0 },
		{ xpos + w, ypos,       1.0, 1.0 },
		{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void renderScene(const Shader &shader, vector<Model> &allModels)
{
	// floor
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	shader.setMat4("model", model);
	renderFloor();

	// 调整人物的位置
	player_model = glm::mat4(1.0f);
	player_model = glm::translate(glm::mat4(1.0f), playerPos);
	player_model = glm::scale(player_model, glm::vec3(0.05f, 0.05f, 0.05f));
	player_model = glm::rotate(player_model, glm::radians(180.0f + rotateAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	shader.setMat4("model", player_model);
	allModels[0].Draw(shader);

	// 调整树的位置
	glm::mat4 treemodel1 = glm::mat4(1.0f);
	treemodel1 = glm::translate(treemodel1, treePos1);
	treemodel1 = glm::scale(treemodel1, glm::vec3(0.05f, 0.05f, 0.05f));
	shader.setMat4("model", treemodel1);
	allModels[1].Draw(shader);

	glm::mat4 treemodel2 = glm::mat4(1.0f);
	treemodel2 = glm::translate(treemodel2, treePos2);
	treemodel2 = glm::scale(treemodel2, glm::vec3(0.05f, 0.05f, 0.05f));
	shader.setMat4("model", treemodel2);
	allModels[1].Draw(shader);

	glm::mat4 treemodel3 = glm::mat4(1.0f);
	treemodel3 = glm::translate(treemodel3, treePos3);
	treemodel3 = glm::scale(treemodel3, glm::vec3(0.05f, 0.05f, 0.05f));
	shader.setMat4("model", treemodel3);
	allModels[1].Draw(shader);

	glm::mat4 treemodel4 = glm::mat4(1.0f);
	treemodel4 = glm::translate(treemodel4, treePos4);
	treemodel4 = glm::scale(treemodel4, glm::vec3(0.05f, 0.05f, 0.05f));
	shader.setMat4("model", treemodel4);
	allModels[1].Draw(shader);

	// 调整房屋的位置
	glm::mat4 house_model = glm::mat4(1.0f);
	house_model = glm::translate(house_model, housePos1);
	house_model = glm::scale(house_model, glm::vec3(0.0025f, 0.0025f, 0.0025f));
	shader.setMat4("model", house_model);
	allModels[2].Draw(shader);

	glm::mat4 house_model1 = glm::mat4(1.0f);
	house_model1 = glm::translate(house_model1, housePos2);
	house_model1 = glm::scale(house_model1, glm::vec3(0.0025f, 0.0025f, 0.0025f));
	shader.setMat4("model", house_model1);
	allModels[2].Draw(shader);

	glm::mat4 house_model2 = glm::mat4(1.0f);
	house_model2 = glm::translate(house_model2, housePos3);
	house_model2 = glm::scale(house_model2, glm::vec3(0.0025f, 0.0025f, 0.0025f));
	shader.setMat4("model", house_model2);
	allModels[3].Draw(shader);

	glm::mat4 house_model3 = glm::mat4(1.0f);
	house_model3 = glm::translate(house_model3, housePos4);
	house_model3 = glm::scale(house_model3, glm::vec3(0.0025f, 0.0025f, 0.0025f));
	shader.setMat4("model", house_model3);
	allModels[3].Draw(shader);
}

void renderFloor()
{
	float planeVertices[48] = {
		// positions            // normals         // texcoords
		25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};

	if (planeVAO == 0) {
		// plane VAO
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
	}
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void renderLight() {
	if (lightVAO == 0) {
		float vertices[] = {
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
			0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
			0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
			0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

			0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
			0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
			0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
			0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
			0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
			0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
			0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
			0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
			0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
		};

		glGenVertexArrays(1, &lightVAO);
		glGenBuffers(1, &lightVBO);
		// Fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindVertexArray(lightVAO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	glBindVertexArray(lightVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	// WASD up down控制视角移动
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		(*camera).ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		(*camera).ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		(*camera).ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		(*camera).ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		(*camera).ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		(*camera).ProcessKeyboard(DOWN, deltaTime);
	// IJKL控制人物的前后左右移动
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		playerMovement(GLFW_KEY_I);
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		playerMovement(GLFW_KEY_K);
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		playerMovement(GLFW_KEY_J);
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		playerMovement(GLFW_KEY_L);
	//更换摄像机模式
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
		if (followPlayer != true) {
			followPlayer = true;
			camera = &playerCamera;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
		if (followPlayer != false) {
			followPlayer = false;
			camera = &viewCamera;
		}
	}
	//Gamma
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
		if (isGamma != true) {
			isGamma = true;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		if (isGamma == true) {
			isGamma = false;
		}
	}
}

void playerMovement(int key) {
	// IJKL控制人物的前后左右移动
	if (key == GLFW_KEY_I) {
		playerPos += glm::vec3(-0.03f * glm::sin((rotateAngle / 180) * pi_2), 0.0f, -0.03f * glm::cos((rotateAngle / 180) * pi_2));
	}
	if (key == GLFW_KEY_K) {
		playerPos += glm::vec3(0.03f * glm::sin((rotateAngle / 180) * pi_2), 0.0f, 0.03f * glm::cos((rotateAngle / 180) * pi_2));
	}
	if (key == GLFW_KEY_J) {
		playerPos += glm::vec3(-0.03f * glm::cos((rotateAngle / 180) * pi_2), 0.0f, 0.03f * glm::sin((rotateAngle / 180) * pi_2));
	}
	if (key == GLFW_KEY_L) {
		playerPos += glm::vec3(0.03f * glm::cos((rotateAngle / 180) * pi_2), 0.0f, -0.03f * glm::sin((rotateAngle / 180) * pi_2));
	}
	if (checkCollision()) {
		// IJKL控制人物的前后左右移动
		if (key == GLFW_KEY_I) {
			playerPos -= glm::vec3(-0.03f * glm::sin((rotateAngle / 180) * pi_2), 0.0f, -0.03f * glm::cos((rotateAngle / 180) * pi_2));
		}
		if (key == GLFW_KEY_K) {
			playerPos -= glm::vec3(0.03f * glm::sin((rotateAngle / 180) * pi_2), 0.0f, 0.03f * glm::cos((rotateAngle / 180) * pi_2));
		}
		if (key == GLFW_KEY_J) {
			playerPos -= glm::vec3(-0.03f * glm::cos((rotateAngle / 180) * pi_2), 0.0f, 0.03f * glm::sin((rotateAngle / 180) * pi_2));
		}
		if (key == GLFW_KEY_L) {
			playerPos -= glm::vec3(0.03f * glm::cos((rotateAngle / 180) * pi_2), 0.0f, -0.03f * glm::sin((rotateAngle / 180) * pi_2));
		}
	}
	//cout << playerPos[0] << " " << playerPos[1] << " " << playerPos[2] << endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (!KEY_DOWN(VK_RBUTTON))
	{
		lastX = xpos;
		lastY = ypos;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	(*camera).ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	(*camera).ProcessMouseScroll(yoffset);
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

void renderSkybox() {
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

		-25.0f,  25.0f, -25.0f,
		25.0f,  25.0f, -25.0f,
		25.0f,  25.0f,  25.0f,
		25.0f,  25.0f,  25.0f,
		-25.0f,  25.0f,  25.0f,
		-25.0f,  25.0f, -25.0f,

		-25.0f, -25.0f, -25.0f,
		-25.0f, -25.0f,  25.0f,
		25.0f, -25.0f, -25.0f,
		25.0f, -25.0f, -25.0f,
		-25.0f, -25.0f,  25.0f,
		25.0f, -25.0f,  25.0f
	};

	if (skyboxVAO == 0) {
		glGenVertexArrays(1, &skyboxVAO);
		glGenBuffers(1, &skyboxVBO);
		glBindVertexArray(skyboxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	}
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

}

void changePlayerFaceTo() {
	if (isFirstFrame) {
		(*camera).refreshCamera(lastRotateFront, getCameraFollowPlayer());
		isFirstFrame = false;
		return;
	}
	glm::vec2 baseVec = glm::normalize(glm::vec2(0.0f, -1.0f));
	glm::vec2 faceVec = glm::normalize(glm::vec2((*camera).Front[0], (*camera).Front[2]));

	float dot = baseVec[0] * faceVec[0] + baseVec[1] * faceVec[1];
	float angle = (glm::acos(dot) / pi_2) * 180;

	float cross = baseVec[0] * faceVec[1] - baseVec[1] * faceVec[0];

	if (cross > 0) {
		angle = 360 - angle;
	}
	rotateAngle = angle;
}

glm::vec3 getCameraFollowPlayer() {
	return playerPos + glm::vec3(1.5 * glm::sin((rotateAngle / 180) * pi_2), 1.0f, 1.5 * glm::cos((rotateAngle / 180) * pi_2));
}