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
    VkResult CreateSurface();
    bool PickPhysicalDevice();
    VkResult CreateLogicalDevice();
    VkResult CreateSwapChain();
    void RetrieveQueueHandles();
    void RetrieveSwapChainImages();
    VkResult CreateSwapChainImageViews();
    VkResult CreateRenderPass();
    VkResult CreateGraphicsPipeline();
    VkResult CreateSwapChainFrameBuffers();
    VkResult CreateCommandPool();
    VkResult CreateCommandBuffer();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void DrawFrame();
    VkResult CreateSyncObjects();

    int m_Width;
    int m_Height;
    GLFWwindow* m_Window;
    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    VkSurfaceKHR m_Surface;
    std::vector<const char*> m_InstanceValidationLayerNames;
    std::vector<const char*> m_InstanceExtensionNames;
    std::vector<const char*> m_PhysicalDeviceExtensionNames;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    VkQueue m_GrahicsQueue;
    VkQueue m_PresentQueue;
    VkSwapchainKHR m_SwapChain;
    std::vector<VkImage> m_SwapChainImages;
    VkFormat m_ImageFormat;
    VkExtent2D m_ImageExtend;
    std::vector<VkImageView> m_SwapChainImageViews;
    VkShaderModule m_VertexShader;
    VkShaderModule m_FragmentShader;
    VkRenderPass m_RenderPass;
    VkPipelineLayout m_PipeLineLayout;
    VkPipeline m_PipeLine;
    std::vector<VkFramebuffer> m_SwapChainFrameBuffers;
    VkCommandPool m_CommandPool;
    VkCommandBuffer m_CommandBuffer;
    VkSemaphore m_ImageAvailable;
    VkSemaphore m_RenderFinished;
    VkFence m_InFlight;
};

#endif