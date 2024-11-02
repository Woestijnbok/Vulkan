#ifndef APPLICATION
#define APPLICATION

#include <vulkan.hpp>
#include <vector>

#include "HelperStructs.h"

class Mesh;
class Texture;
struct GLFWwindow;
class Camera;

void GlobalKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

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
    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

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
    VkResult CreateGraphicsPipeline();
    VkResult CreateSwapChainFrameBuffers();
    VkResult CreateCommandPool();
    VkResult CreateCommandBuffers();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void UpdateUniformBuffers(uint32_t currentImage);
    void DrawFrame();
    VkResult CreateSyncObjects();
    void RecreateSwapChain();
    void CleanupSwapChain();
    VkResult CreateUniformBuffers();
    VkResult CreateDescriptorPool();
    VkResult CreateTexturesDescriptorSetLayout();
    VkResult CreateTransformsDescriptorSetLayout();
    VkResult CreateTexturesDescriptorSets();
    VkResult CreateTransformsDescriptorSets();
    void CreateTextureSampler();
    void CreateDepthResources();
    void CreateColorResources();
    void InitializeTextures();

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
    std::vector<VkCommandBuffer> m_CommandBuffers;
    std::vector<VkSemaphore> m_ImageAvailable;
    std::vector<VkSemaphore> m_RenderFinished;
    std::vector<VkFence> m_InFlight;
    uint32_t m_CurrentFrame;
    bool m_FrameBufferResized;
    std::vector<Mesh*> m_Meshes;
    std::vector< std::vector<VkBuffer>> m_UniformBuffers;
    std::vector< std::vector<VkDeviceMemory>> m_UniformBufferMemories;
    std::vector< std::vector<void*>> m_UniformBufferMaps;
    VkDescriptorSetLayout m_TexturesDescriptorSetLayout;
    VkDescriptorSetLayout m_TransformsDescriptorSetLayout;
    VkDescriptorPool m_DescriptorPool;
    std::vector<VkDescriptorSet> m_TexturesDescriptorSets;
    std::vector< std::vector<VkDescriptorSet>> m_TransformsDescriptorSets;
    std::vector<Texture*> m_BaseColorTextures;
    std::vector<Texture*> m_NormalTextures;
    std::vector<Texture*> m_GlossTextures;
    std::vector<Texture*> m_SpecularTextures;
    VkSampler m_TextureSampler;
    VkImage m_DepthImage;
    VkDeviceMemory m_DepthMemory;
    VkImageView m_DepthImageView;
    VkImage m_ColorImage;
    VkDeviceMemory m_ColorMemory;
    VkImageView m_ColorImageView;
    Camera* m_Camera;
    VkSampleCountFlagBits m_MSAASamples;
    PushConstants m_PushConstants;                                 
};

#endif