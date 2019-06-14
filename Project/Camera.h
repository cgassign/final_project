#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


class Camera
{
public:
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	float Yaw;					// 欧拉角的偏航角 左右
	float Pitch;				// 欧拉角的俯仰角 上下

	
	float MovementSpeed;		//移动速度
	float MouseSensitivity;		// 鼠标灵敏度
	float Zoom;					// 缩放

	//构造函数1
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
	//构造函数2
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);
	// 获取view矩阵
	glm::mat4 GetViewMatrix();
	//获取键盘输入
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);
	// 鼠标移动，改变摄像机的欧拉角
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
	// 鼠标缩放，改变视野
	void ProcessMouseScroll(float yoffset);

private:
	void updateCameraVectors();
};