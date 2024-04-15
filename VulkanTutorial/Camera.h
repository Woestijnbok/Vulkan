#ifndef CAMERA
#define CAMERA

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <chrono>

const glm::vec3 g_WorldForward{ -1.0f, 0.0f, 0.0f };
const glm::vec3 g_WorldRight{ 0.0f, 1.0f, 0.0f };
const glm::vec3 g_WorldUp{ 0.0f, 0.0f, 1.0f };

class Camera
{
public:
    Camera(float fieldOfView, float ascpectRatio, float nearPlane, float farPlane);
    ~Camera() = default;

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) = delete;
    Camera& operator=(Camera&&) = delete;

    void Update(GLFWwindow* window, std::chrono::duration<float> seconds);
    glm::mat4 GetViewMatrx() const;
    glm::mat4 GetProjectionMatrix() const;

private:
    const float m_MovementSpeed;
    const float m_RotationSensitivity;
    const glm::mat4 m_ProjectionMatrix;
    glm::mat4 m_ViewMatrix;
    glm::vec3 m_Position;
    glm::vec3 m_Forward;
    glm::vec3 m_Right;
    glm::vec3 m_Up;
    glm::vec2 m_LastMousePosition;
    float m_Yaw;
    float m_Pitch;
    bool m_RightMouseButtonPressed;

    void HandleCameraMovement(GLFWwindow* window, std::chrono::duration<float> seconds);
    void HandleCameraRotation(GLFWwindow* window);
    void CalculateViewMatrix();
};

#endif