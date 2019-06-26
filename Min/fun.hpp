#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "SOIL.h"

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

// ¶¨Òå×Ö·û
struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint Advance;    // Horizontal offset to advance to next glyph
};

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
unsigned int loadTexture(char const * path);
void renderScene(const Shader &shader, vector<Model> &allModels);
void renderFloor();
void renderLight();

unsigned int loadCubemap(vector<std::string> faces);
void renderSkybox();

