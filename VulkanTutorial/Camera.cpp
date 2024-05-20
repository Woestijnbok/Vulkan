#include "Camera.h"

#include <glm/vec3.hpp> 
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp> 
#include <glm/ext/matrix_transform.hpp> 
#include <glm/ext/matrix_clip_space.hpp> 
#include <glm/ext/scalar_constants.hpp>
#include <iostream>
#include <format>

Camera::Camera(float fieldOfView, float ascpectRatio, float nearPlane, float farPlane, float movementSpeed) :
    m_MovementSpeed{ movementSpeed },
    m_RotationSensitivity{ 0.01f },
    m_ProjectionMatrix{ glm::perspective(fieldOfView, ascpectRatio, nearPlane, farPlane) },
    m_ViewMatrix{},
    m_Position{},
    m_Forward{},
    m_Right{},
    m_Up{},
    m_LastMousePosition{},
    m_Yaw{},
    m_Pitch{},
    m_RightMouseButtonPressed{ false }
{
    CalculateViewMatrix();

    std::cout << "--- Camera Controls ---" << std::endl;
    std::cout << "Movement forward / backwards with W & S" << std::endl;
    std::cout << "Movement right / left with D & A" << std::endl;
    std::cout << "Movement up / down with E & Q" << std::endl;
    std::cout << "Rotate yaw with right click drag x" << std::endl;
    std::cout << "Rotate pitch with right click drag y" << std::endl << std::endl;
}

void Camera::Update(GLFWwindow* window, std::chrono::duration<float> seconds)
{
    HandleCameraMovement(window, seconds);
    HandleCameraRotation(window);
    CalculateViewMatrix();  
}

glm::mat4 Camera::GetViewMatrx() const
{
    return m_ViewMatrix;
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    return m_ProjectionMatrix;
}

void Camera::SetStartPosition(const glm::vec3& position, float yaw, float pitch)    
{
    m_Position = position;
    m_Yaw = yaw;
    m_Pitch = pitch;

    CalculateViewMatrix();
}

glm::vec3 Camera::GetPosition() const
{
    return m_Position;
}

void Camera::HandleCameraMovement(GLFWwindow* window, std::chrono::duration<float> seconds)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        m_Position += m_Forward * m_MovementSpeed * seconds.count();
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        m_Position -= m_Forward * m_MovementSpeed * seconds.count();
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        m_Position += m_Up * m_MovementSpeed * seconds.count();
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        m_Position -= m_Up * m_MovementSpeed * seconds.count();
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        m_Position -= m_Right * m_MovementSpeed * seconds.count();
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        m_Position += m_Right * m_MovementSpeed * seconds.count();
    }
}

void Camera::HandleCameraRotation(GLFWwindow* window)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        double xPosition{}, yPosition{};
        glfwGetCursorPos(window, &xPosition, &yPosition);

        if (!m_RightMouseButtonPressed)
        {
            m_LastMousePosition.x = float(xPosition);
            m_LastMousePosition.y = float(yPosition);

            m_RightMouseButtonPressed = true;
        }
        else
        {
            const float xOffset{ m_LastMousePosition.x - float(xPosition) };
            const float yOffset{ m_LastMousePosition.y - float(yPosition) };

            m_Yaw += xOffset * m_RotationSensitivity;
            m_Pitch += yOffset * m_RotationSensitivity;
            m_Yaw = fmodf(m_Yaw, 2 * glm::pi<float>());
            m_Pitch = std::clamp(m_Pitch, -glm::pi<float>() / 2 + 0.0001f, glm::pi<float>() / 2 - 0.0001f);

            m_LastMousePosition.x = float(xPosition);
            m_LastMousePosition.y = float(yPosition);
        }
    }
    else
    {
        m_RightMouseButtonPressed = false;
    }
}

void Camera::CalculateViewMatrix()
{
    glm::mat4x4 rotation = glm::rotate(glm::mat4x4(1.f), m_Yaw, g_WorldUp);
    rotation = glm::rotate(rotation, m_Pitch, g_WorldRight);

    m_Forward = glm::vec3{ rotation * glm::vec4{ g_WorldForward, 0.0f } };
    m_Right = glm::vec3{ rotation * glm::vec4{ g_WorldRight, 0.0f } };
    m_Up = glm::vec3{ rotation * glm::vec4{ g_WorldUp, 0.0f } };

    m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Forward, m_Up);

    //std::cout << std::format("Position: {}, {}, {} Pitch: {} Yaw: {}", m_Position.x, m_Position.y, m_Position.z, m_Pitch, m_Yaw) << std::endl;
}