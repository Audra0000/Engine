#pragma once
#include <glm/glm.hpp>
#include "glm/gtc/quaternion.hpp"

class Camera
{
public:
	Camera();
	~Camera();

	void Update();

	const glm::mat4& GetViewMatrix() const { return viewMatrix; }
	const glm::mat4& GetProjectionMatrix() const { return projectionMatrix; }

	glm::vec3 GetPosition() const { return cameraPos; }
	glm::vec3 GetFront() const { return cameraFront; }
	glm::vec3 GetUp() const { return cameraUp; }
	void SetPosition(const glm::vec3& newPosition) { cameraPos = newPosition; }
	void HandleMouseInput(float xpos, float ypos);
	void ResetMouseInput() { firstMouse = true; }

private:
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	float yaw;
	float pitch;
	float lastX;
	float lastY;
	bool firstMouse;

	void UpdateCameraVectors();
	void ProcessMouseMovement(float xoffset, float yoffset);
};