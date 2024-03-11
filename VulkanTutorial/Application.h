#ifndef APPLICATION
#define APPLICATION

#include <vulkan/vulkan.h>

struct GLFWwindow;

class Application
{
public:

    Application(int width, int height);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

	void Run();

private:

    void InitializeWindow();
    void InitializeVulkan();
    bool GLFWExtensionsPresent();
    VkResult CreateVulkanInstance();

    int m_Width;
    int m_Height;
    GLFWwindow* m_Window;
    VkInstance m_Instance;
};

#endif