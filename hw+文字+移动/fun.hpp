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
Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

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

glm::vec3 lightPos(2.0f, 4.0f, -3.0f);

glm::mat4 player_model = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, 0.0f)), glm::vec3(0.05f, 0.05f, 0.05f));

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
	// 地面
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
	shader.setMat4("model", model);
	renderFloor();

	// 调整树的位置
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(6.0f, -1.5f, -4.0f));
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	shader.setMat4("model", model);
	allModels[1].Draw(shader);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(1.0f, -1.5f, -8.0f));
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	shader.setMat4("model", model);
	allModels[1].Draw(shader);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.5f, -1.5f, -3.5f));
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	shader.setMat4("model", model);
	allModels[1].Draw(shader);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(3.5f, -1.5f, -3.5f));
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	shader.setMat4("model", model);
	allModels[1].Draw(shader);

	// 调整人物的位置
	player_model = glm::rotate(player_model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	shader.setMat4("model", player_model);
	allModels[0].Draw(shader);

	// 调整房屋的位置
	glm::mat4 house_model = glm::mat4(1.0f);
	house_model = glm::translate(house_model, glm::vec3(5.0f, -1.5f, -8.0f));
	house_model = glm::scale(house_model, glm::vec3(0.0025f, 0.0025f, 0.0025f));
	shader.setMat4("model", house_model);
	allModels[2].Draw(shader);
	glm::mat4 house_model1 = glm::mat4(1.0f);
	house_model1 = glm::translate(house_model1, glm::vec3(-1.0f, -1.5f, -10.0f));
	house_model1 = glm::scale(house_model1, glm::vec3(0.0025f, 0.0025f, 0.0025f));
	shader.setMat4("model", house_model1);
	allModels[2].Draw(shader);

	glm::mat4 house_model2 = glm::mat4(1.0f);
	house_model2 = glm::translate(house_model2, glm::vec3(-3.0f, -1.5f, -5.0f));
	house_model2 = glm::scale(house_model2, glm::vec3(0.0025f, 0.0025f, 0.0025f));
	shader.setMat4("model", house_model2);
	allModels[3].Draw(shader);
	glm::mat4 house_model3 = glm::mat4(1.0f);
	house_model3 = glm::translate(house_model3, glm::vec3(2.0f, -1.5f, -6.0f));
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
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	// IJKL控制人物的前后左右移动
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		player_model = glm::translate(player_model, glm::vec3(0.0f, 0.0f, -0.3f));
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		player_model = glm::translate(player_model, glm::vec3(0.0f, 0.0f, 0.3f));
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		player_model = glm::translate(player_model, glm::vec3(-0.3f, 0.0f, 0.0f));;
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		player_model = glm::translate(player_model, glm::vec3(0.3f, 0.0f, 0.0f));
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
		firstMouse = false;
	}

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