#ifndef APPLICATION
#define APPLICATION

#include <vulkan/vulkan.hpp>
#include <vector>

class Mesh;
class Texture;
struct GLFWwindow;
class Camera;

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
    static void FrameBufferResizedCallback(GLFWwindow* window, int width, int height);

private:

    void InitializeMeshes();
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
    VkResult CreateDescriptorSetLayout();
    VkResult CreateGraphicsPipeline();
    VkResult CreateSwapChainFrameBuffers();
    VkResult CreateCommandPool();
    VkResult CreateCommandBuffers();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void UpdateUniformBuffer(uint32_t currentImage, Mesh* mesh);
    void DrawFrame();
    VkResult CreateSyncObjects();
    void RecreateSwapChain();
    void CleanupSwapChain();
    VkResult CreateUniformBuffers();
    VkResult CreateDescriptorPool();
    VkResult CreateDescriptorSets();
    void CreateTextureSampler();
    void CreateDepthResources();
    void CreateColorResources();

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
    VkDescriptorSetLayout m_DescriptorSetLayout;
    VkPipelineLayout m_PipeLineLayout;
    VkPipeline m_PipeLine;
    std::vector<VkFramebuffer> m_SwapChainFrameBuffers;
    VkCommandPool m_CommandPool;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    std::vector<VkSemaphore> m_ImageAvailable;
    std::vector<VkSemaphore> m_RenderFinished;
    std::vector<VkFence> m_InFlight;
    uint32_t m_CurrentFrame;
    bool m_FrameBufferResized;
    std::vector<Mesh*> m_Meshes;
    std::vector<VkBuffer> m_UniformBuffers;
    std::vector<VkDeviceMemory> m_UniformBufferMemories;
    std::vector<void*> m_UniformBufferMaps;
    VkDescriptorPool m_DescriptorPool;
    std::vector<VkDescriptorSet> m_DescriptorSets;
    Texture* m_BaseColorTexture;
    Texture* m_NormalTexture;
    Texture* m_MetalnessTexture;
    Texture* m_RoughnessTexture;
    Texture* m_AmbientOcclusionTexture;
    VkSampler m_TextureSampler;
    VkImage m_DepthImage;
    VkDeviceMemory m_DepthMemory;
    VkImageView m_DepthImageView;
    VkImage m_ColorImage;
    VkDeviceMemory m_ColorMemory;
    VkImageView m_ColorImageView;
    Camera* m_Camera;
    VkSampleCountFlagBits m_MSAASamples;
};

#endif