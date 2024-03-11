#ifndef APPLICATION
#define APPLICATION

#include <vulkan/vulkan.hpp>
#include <vector>

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
    bool ExtensionsPresent();
    bool ValidationLayersPresent();
    VkResult CreateVulkanInstance();
    VkResult SetupDebugMessenger();

    int m_Width;
    int m_Height;
    GLFWwindow* m_Window;
    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    std::vector<const char*> m_ValidationLayerNames;
    std::vector<const char*> m_ExtensionNames;
};

#endif