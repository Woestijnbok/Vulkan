#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <format>

Camera::Camera(float fieldOfView, float ascpectRatio, float nearPlane, float farPlane) :
    m_MovementSpeed{ 2.5f },
    m_RotationSensitivity{ 0.1f },
    m_ProjectionMatrix{ glm::perspective(fieldOfView, ascpectRatio, nearPlane, farPlane) }, 
    m_Position{ 2.0f, 2.0f, 2.0f },
    m_Forward{ -1.0f, 0.0f, 0.0f },
    m_Up{ 0.0f, 0.0f, 1.0f },
    m_LastMousePosition{},
    m_Pitch{},
    m_Yaw{},
    m_FirstProcess{ true }
{
    std::cout << "--- Camera Controls ---" << std::endl;
    std::cout << "Movement forward / backwards with W & S" << std::endl;
    std::cout << "Movement right / left with D & A" << std::endl;
    std::cout << "Movement up / down with E & Q" << std::endl;
}

void Camera::ProcessInput(GLFWwindow* window, std::chrono::duration<float> seconds)
{
    window;
    seconds;

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
        m_Position -= glm::normalize(glm::cross(m_Forward, m_Up)) * m_MovementSpeed * seconds.count();
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        m_Position += glm::normalize(glm::cross(m_Forward, m_Up)) * m_MovementSpeed * seconds.count();  
    }    

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    if (m_FirstProcess) 
    {
       m_LastMousePosition.x = float(xpos);
       m_LastMousePosition.y = float(ypos);
       m_FirstProcess = false;
    }

    float xoffset = m_LastMousePosition.x - float(xpos);
    float yoffset = m_LastMousePosition.y - float(ypos); // reversed since y-coordinates go from bottom to top
    m_LastMousePosition.x = float(xpos);
    m_LastMousePosition.y = float(ypos);

    xoffset *= m_RotationSensitivity;
    yoffset *= m_RotationSensitivity;

    m_Yaw += yoffset;
    m_Pitch += xoffset;

    UpdateCameraAxis();
}

glm::mat4 Camera::GetViewMatrx() const
{
    //return glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    return glm::lookAt(m_Position, glm::vec3(0.0f, 0.0f, 0.0f), m_Up);
    //return glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    return m_ProjectionMatrix;
}

void Camera::UpdateCameraAxis()
{
    const glm::vec3 newFront
    {
        cos(glm::radians(m_Yaw))* cos(glm::radians(m_Pitch)),
        sin(glm::radians(m_Pitch)),
        -sin(glm::radians(m_Yaw))* cos(glm::radians(m_Pitch))
    };
    
    m_Forward = glm::normalize(newFront);
}