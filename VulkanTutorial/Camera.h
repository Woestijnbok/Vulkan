#ifndef CAMERA
#define CAMERA

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <chrono>

class Camera
{
public:
    Camera(float fieldOfView, float ascpectRatio, float nearPlane, float farPlane);
    ~Camera() = default;

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) = delete;
    Camera& operator=(Camera&&) = delete;

    void ProcessInput(GLFWwindow* window, std::chrono::duration<float> seconds);
    glm::mat4 GetViewMatrx() const;
    glm::mat4 GetProjectionMatrix() const;

private:
    const float m_MovementSpeed;
    const float m_RotationSensitivity;
    const glm::mat4 m_ProjectionMatrix;
    glm::vec3 m_Position;
    glm::vec3 m_Forward;
    glm::vec3 m_Up;
    glm::vec2 m_LastMousePosition;
    float m_Pitch;
    float m_Yaw;
    bool m_FirstProcess;

    void UpdateCameraAxis();
};

#endif