#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS

#ifdef NDEBUG
const bool g_EnableValidationlayers{ false };
#else
const bool g_EnableValidationlayers{ true };
#endif

const int g_MaxFramePerFlight{ 2 };

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <cstring>
#include <set>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#include "Application.h"
#include "HelperFunctions.h"
#include "Mesh.h"

Application::Application(int width, int height) :
	m_Width{ width },
	m_Height{ height },
	m_Window{ nullptr },
	m_Instance{},
	m_DebugMessenger{},
	m_Surface{},
	m_InstanceValidationLayerNames{ "VK_LAYER_KHRONOS_validation" },	// Since we are already checking for presentation queue family this will also be checked
	m_InstanceExtensionNames{},
	m_PhysicalDeviceExtensionNames{ VK_KHR_SWAPCHAIN_EXTENSION_NAME },
	m_PhysicalDevice{ VK_NULL_HANDLE },
	m_Device{ VK_NULL_HANDLE },
	m_GrahicsQueue{ VK_NULL_HANDLE },
	m_PresentQueue{ VK_NULL_HANDLE },
	m_SwapChainImages{},
	m_SwapChainImageViews{},
	m_VertexShader{},
	m_FragmentShader{},
	m_RenderPass{},
	m_DescriptorSetLayout{},
	m_PipeLineLayout{},
	m_PipeLine{},
	m_SwapChainFrameBuffers{},
	m_CommandPool{},
	m_CommandBuffers{},
	m_ImageAvailable{},
	m_RenderFinished{},
	m_InFlight{},
	m_CurrentFrame{},
	m_FrameBufferResized{ false },
	m_Mesh{ nullptr },
	m_UniformBuffers{},
	m_UniformBufferMemories{},
	m_UniformBufferMaps{},
	m_DescriptorPool{},
	m_DescriptorSets{}
{
	InitializeWindow();
	InitializeVulkan();
	InitializeMesh();
}

Application::~Application()
{
	for (int i{}; i < g_MaxFramePerFlight; ++i)
	{
		vkDestroyBuffer(m_Device, m_UniformBuffers.at(i), nullptr);
		vkFreeMemory(m_Device, m_UniformBufferMemories.at(i), nullptr);
	}
	delete m_Mesh;
	for (int i{}; i < g_MaxFramePerFlight; ++i)
	{
		vkDestroySemaphore(m_Device, m_ImageAvailable[i], nullptr);
		vkDestroySemaphore(m_Device, m_RenderFinished[i], nullptr);
		vkDestroyFence(m_Device, m_InFlight[i], nullptr);
	}
	vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
	CleanupSwapChain();
	vkDestroyPipeline(m_Device, m_PipeLine, nullptr);
	vkDestroyPipelineLayout(m_Device, m_PipeLineLayout, nullptr);
	vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
	vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
	vkDestroyShaderModule(m_Device, m_VertexShader, nullptr);
	vkDestroyShaderModule(m_Device, m_FragmentShader, nullptr);
	vkDestroyDevice(m_Device, nullptr);
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	if (g_EnableValidationlayers) DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
	vkDestroyInstance(m_Instance, nullptr);
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void Application::Run()
{
	while (!glfwWindowShouldClose(m_Window))
	{
		glfwPollEvents();
		DrawFrame();
	}

	vkDeviceWaitIdle(m_Device);
}

void Application::InitializeMesh()
{
	const std::vector<Vertex> vertices
	{
		Vertex{ glm::vec2{ -0.5f, -0.5f }, glm::vec3{ 1.0f, 0.0f, 0.0f } },
		Vertex{ glm::vec2{ 0.5f, -0.5f }, glm::vec3{ 0.0f, 1.0f, 0.0f } },
		Vertex{ glm::vec2{ 0.5f, 0.5f }, glm::vec3{ 0.0f, 0.0f, 1.0f } },
		Vertex{ glm::vec2{ -0.5f, 0.5f }, glm::vec3{ 1.0f, 1.0f, 1.0f } }
	};

	const std::vector<uint16_t> indices
	{
		0, 1, 2,	// triangle 1
		2, 3, 0		// triangle 2
	};

	// Graphics queue can handle copy commands, you could create a seperate command pool for copying buffers
	m_Mesh = new Mesh{ m_PhysicalDevice, m_Device, m_CommandPool, m_GrahicsQueue, vertices, indices };
}

void Application::InitializeWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	m_Window = glfwCreateWindow(m_Width, m_Height, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(m_Window, this);
	glfwSetFramebufferSizeCallback(m_Window, &FrameBufferResizedCallback);
	
	if (m_Window == nullptr) throw std::runtime_error("failed to create window!");
}

void Application::InitializeVulkan()
{
	if (!ExtensionsPresent())
	{
		throw std::runtime_error("vulkan doesn't have the required extensions for glfw!");
	}

	if (g_EnableValidationlayers)
	{
		if (!ValidationLayersPresent())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}
	}


	if (CreateVulkanInstance() != VK_SUCCESS) throw std::runtime_error("failed to create vulkan instance!");
	if (SetupDebugMessenger() != VK_SUCCESS) throw std::runtime_error("failed to setup debug messenger!");
	if (CreateSurface() != VK_SUCCESS) throw std::runtime_error("failed to create surface!");
	if (!PickPhysicalDevice()) throw std::runtime_error("Failed to find suitable gpu!");
	if (CreateLogicalDevice() != VK_SUCCESS) throw std::runtime_error("failed to create logical device!");
	RetrieveQueueHandles();
	if (CreateSwapChain() != VK_SUCCESS) throw std::runtime_error("failed to create swap chain!");
	RetrieveSwapChainImages();
	if (CreateSwapChainImageViews() != VK_SUCCESS) throw std::runtime_error("failed to create swap chain image views!");
	if (CreateRenderPass() != VK_SUCCESS) throw std::runtime_error("failed to create render pass!");
	if (CreateDescriptorSetLayout() != VK_SUCCESS) throw std::runtime_error("failed to create descriptor set layout!");
	if (CreateGraphicsPipeline() != VK_SUCCESS) throw std::runtime_error("failed to create grahpics pipeline!");
	if (CreateSwapChainFrameBuffers() != VK_SUCCESS) throw std::runtime_error("failed to create swap chain frame buffers!");
	if (CreateCommandPool() != VK_SUCCESS) throw std::runtime_error("failed to create command pool!");
	if (CreateCommandBuffers() != VK_SUCCESS) throw std::runtime_error("failed to create command buffer!");
	if (CreateSyncObjects() != VK_SUCCESS) throw std::runtime_error("failed to create sync objects!");
	if (CreateUniformBuffers() != VK_SUCCESS) throw std::runtime_error("failed to create uniform buffers!");
	if (CreateDescriptorPool() != VK_SUCCESS) throw std::runtime_error("failed to create descriptor pool!");
	if (CreateDescriptorSets() != VK_SUCCESS) throw std::runtime_error("failed to create descriptor sets!");
}

bool Application::ExtensionsPresent()
{
	uint32_t vulkanExtensionCount{};
	vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(vulkanExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, extensions.data());

	uint32_t glfwExtensionCount{};
	const char** glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };
	m_InstanceExtensionNames = std::vector<const char*>{ glfwExtensions, glfwExtensions + glfwExtensionCount };
	if (g_EnableValidationlayers) m_InstanceExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	bool extensionsPresent{ true };
	std::cout << std::endl << "-----Instance Extensions-----" << std::endl;
	for (const auto& extensionName : m_InstanceExtensionNames)
	{
		std::cout << std::setw(40) << std::left << extensionName;

		extensions.at(0).extensionName;

		if (std::ranges::find_if(extensions,
			[&extensionName](const auto& extension) -> bool
			{
				return strcmp(extensionName, extension.extensionName) == 0;
			}
		) != extensions.end())
		{
			std::cout << std::setw(40) << std::left << "PRESENT";
		}
		else
		{
			extensionsPresent = false;
			std::cout << std::setw(40) << std::left << "NOT PRESENT";
		}

		std::cout << std::endl;
	}
	std::cout << std::endl;

	return extensionsPresent;
}

bool Application::ValidationLayersPresent()
{
	uint32_t validationLayerCount{};
	vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);
	std::vector<VkLayerProperties> validationLayers(validationLayerCount);
	vkEnumerateInstanceLayerProperties(&validationLayerCount, validationLayers.data());

	bool validationLayersPresent{ true };
	std::cout << std::endl << "-----Instance Validation layers-----" << std::endl;
	for (const auto& validationLayerName : m_InstanceValidationLayerNames)
	{
		std::cout << std::setw(40) << std::left << validationLayerName;

		if (std::ranges::find_if(validationLayers, 
			[&validationLayerName](const auto& validationLayer) -> bool
				{ 
					return strcmp(validationLayerName, validationLayer.layerName) == 0;
				}
			) != validationLayers.end())
		{
			std::cout << std::setw(40) << std::left << "PRESENT";
		}
		else
		{
			validationLayersPresent = false;
			std::cout << std::setw(40) << std::left << "NOT PRESENT";
		}

		std::cout << std::endl;
	}
	std::cout << std::endl;

	return validationLayersPresent;
}

VkResult Application::CreateVulkanInstance()
{
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkApplicationInfo.html
	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = "Hello Triangle";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "No Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_3;

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkInstanceCreateInfo.html
	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_InstanceExtensionNames.size());
	instanceCreateInfo.ppEnabledExtensionNames = m_InstanceExtensionNames.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (g_EnableValidationlayers)
	{
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_InstanceValidationLayerNames.size());
		instanceCreateInfo.ppEnabledLayerNames = m_InstanceValidationLayerNames.data();

		// pNext will contain the debug struct so we can have debugging on vkCreateInstance and vkDestroyInstance()
		FillDebugMessengerCreateInfo(debugCreateInfo);
		instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		instanceCreateInfo.enabledLayerCount = 0;
		instanceCreateInfo.pNext = nullptr;
	}

	return vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);
}

VkResult Application::SetupDebugMessenger()
{
	if (!g_EnableValidationlayers) return VK_SUCCESS;

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDebugUtilsMessengerCreateInfoEXT.html
	VkDebugUtilsMessengerCreateInfoEXT createInformation{};
	FillDebugMessengerCreateInfo(createInformation);

	if (CreateDebugUtilsMessengerEXT(m_Instance, &createInformation, nullptr, &m_DebugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}

	return VK_SUCCESS;
}

VkResult Application::CreateSurface()
{
	return glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface);
}

bool Application::PickPhysicalDevice()
{
	uint32_t deviceCount{};
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

	if (deviceCount == 0) throw std::runtime_error("Failed to find gpu with vulkan support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

	for (auto device : devices)
	{
		if (IsPhysicalDeviceSuitable(device, m_Surface, m_PhysicalDeviceExtensionNames))
		{
			m_PhysicalDevice = device;
			break;
		}
	}

	return m_PhysicalDevice != VK_NULL_HANDLE;
}

VkResult Application::CreateLogicalDevice()
{
	QueueFamilyIndices queueFamilyIndices{ FindQueueFamilies(m_PhysicalDevice, m_Surface) };
	std::vector<VkDeviceQueueCreateInfo> queueFamailyCreateInfos{};
	std::set<uint32_t> queueFamilieIndexes{ queueFamilyIndices.GraphicsFamily.value(), queueFamilyIndices.PresentFamily.value() };

	const float queuePriority{ 1.0f };

	for (auto queueFamilyIndex : queueFamilieIndexes)
	{
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDeviceQueueCreateInfo.html
		VkDeviceQueueCreateInfo queueFamiliyCreateInfo{};
		queueFamiliyCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueFamiliyCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueFamiliyCreateInfo.queueCount = 1;
		queueFamiliyCreateInfo.pQueuePriorities = &queuePriority;

		queueFamailyCreateInfos.push_back(queueFamiliyCreateInfo);
	}

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceFeatures.html
	VkPhysicalDeviceFeatures physicalDeviceFeatures{};

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDeviceCreateInfo.html
	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueFamailyCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueFamailyCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_PhysicalDeviceExtensionNames.size());
	deviceCreateInfo.ppEnabledExtensionNames = m_PhysicalDeviceExtensionNames.data();
	if (g_EnableValidationlayers)
	{
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_InstanceValidationLayerNames.size());
		deviceCreateInfo.ppEnabledLayerNames = m_InstanceValidationLayerNames.data();
	}
	else
	{
		deviceCreateInfo.enabledLayerCount = 0;
	}
	
	return vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
}

VkResult Application::CreateSwapChain()
{
	SwapChainSupportDetails details{ QuerySwapChainSupportDetails(m_PhysicalDevice, m_Surface) };

	VkSurfaceFormatKHR surfaceFormat{ ChooseSurfaceFormat(details.Formats) };
	VkPresentModeKHR presentMode{ ChoosePresentMode(details.PresentModes) };
	VkExtent2D extend{ ChooseExtent(details.Capabilities, m_Window) };

	m_ImageExtend = extend;
	m_ImageFormat = surfaceFormat.format;

	// Set surface count to min + 1, and make sure we don't exceed max count, if max is set to 0 = no max
	uint32_t surfacesCount{ details.Capabilities.minImageCount + 1 };
	if (details.Capabilities.maxImageCount > 0 && surfacesCount > details.Capabilities.maxImageCount)
	{
		surfacesCount = details.Capabilities.maxImageCount;
	}

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSwapchainCreateInfoKHR.html
	VkSwapchainCreateInfoKHR swapChaincreateInfo{};
	swapChaincreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChaincreateInfo.surface = m_Surface;
	swapChaincreateInfo.minImageCount = surfacesCount;
	swapChaincreateInfo.imageFormat = surfaceFormat.format;
	swapChaincreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChaincreateInfo.imageExtent = extend;
	swapChaincreateInfo.imageArrayLayers = 1;
	swapChaincreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices queueFamilyIndices{ FindQueueFamilies(m_PhysicalDevice, m_Surface) };
	uint32_t rawQueueFamilyIndices[]{ queueFamilyIndices.GraphicsFamily.value(), queueFamilyIndices.PresentFamily.value() };
	if (queueFamilyIndices.GraphicsFamily != queueFamilyIndices.PresentFamily)
	{
		swapChaincreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapChaincreateInfo.queueFamilyIndexCount = 2;
		swapChaincreateInfo.pQueueFamilyIndices = rawQueueFamilyIndices;
	}
	else
	{
		swapChaincreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChaincreateInfo.queueFamilyIndexCount = 0;
		swapChaincreateInfo.pQueueFamilyIndices = nullptr;

	}

	swapChaincreateInfo.preTransform = details.Capabilities.currentTransform;
	swapChaincreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChaincreateInfo.presentMode = presentMode;
	swapChaincreateInfo.clipped = VK_TRUE;
	swapChaincreateInfo.oldSwapchain = VK_NULL_HANDLE;

	return vkCreateSwapchainKHR(m_Device, &swapChaincreateInfo, nullptr, &m_SwapChain);
}

void Application::RetrieveQueueHandles()
{
	QueueFamilyIndices queueFamilyIndices{ FindQueueFamilies(m_PhysicalDevice, m_Surface) };

	vkGetDeviceQueue(m_Device, queueFamilyIndices.GraphicsFamily.value(), 0, &m_GrahicsQueue);
	vkGetDeviceQueue(m_Device, queueFamilyIndices.PresentFamily.value(), 0, &m_PresentQueue);
}

void Application::RetrieveSwapChainImages()
{
	uint32_t imageCount{};
	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
	m_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());
}

VkResult Application::CreateSwapChainImageViews()
{
	m_SwapChainImageViews.resize(m_SwapChainImages.size());
	for (size_t i{}; i < m_SwapChainImages.size(); ++i)
	{
		VkImageViewCreateInfo imageViewcreateInfo{};
		imageViewcreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewcreateInfo.image = m_SwapChainImages.at(i);
		imageViewcreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewcreateInfo.format = m_ImageFormat;
		imageViewcreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewcreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewcreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewcreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewcreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewcreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewcreateInfo.subresourceRange.levelCount = 1;
		imageViewcreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewcreateInfo.subresourceRange.layerCount = 1;

		VkResult result{ vkCreateImageView(m_Device, &imageViewcreateInfo, nullptr, &m_SwapChainImageViews.at(i)) };

		if (result != VK_SUCCESS) return result;
	}

	return VK_SUCCESS;
}

VkResult Application::CreateDescriptorSetLayout()
{
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorSetLayoutBinding.html
	const VkDescriptorSetLayoutBinding uniformBufferObjectDescriptorSetLayoutBinding
	{
		0,											// binding
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			// descriptorType
		1,											// descriptorCount
		VK_SHADER_STAGE_VERTEX_BIT,					// stageFlags
		nullptr										// pImmutableSamplers
	};

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorSetLayoutCreateInfo.html
	const VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,		// sType
		nullptr,													// pNext
		0,															// flags
		1,															// bindingCount
		&uniformBufferObjectDescriptorSetLayoutBinding				// pBindings
	};

	return vkCreateDescriptorSetLayout(m_Device, &descriptorSetLayoutCreateInfo, nullptr, &m_DescriptorSetLayout);
}

VkResult Application::CreateGraphicsPipeline()
{
	
	m_VertexShader = CreateShaderModule(LoadSPIRV("shaders/vert.spv"), m_Device);
	m_FragmentShader = CreateShaderModule(LoadSPIRV("shaders/frag.spv"), m_Device);

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineShaderStageCreateInfo.html
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = m_VertexShader;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = m_FragmentShader;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[2]{ vertShaderStageInfo, fragShaderStageInfo };

	const auto vertexBindingDescription{ Vertex::GetBindingDescription() };
	const auto vertexAttributeDescriptions{ Vertex::GetAttributeDescriptions() };

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineVertexInputStateCreateInfo.html
	const VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo
	{
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,		// sType
		nullptr,														// pNext
		0,																// flags
		1,																// vertexBindingDescriptionCount
		&vertexBindingDescription,										// pVertexBindingDescriptions
		static_cast<uint32_t>(vertexAttributeDescriptions.size()),		// vertexAttributeDescriptionCount
		vertexAttributeDescriptions.data()								// pVertexAttributeDescriptions
	};

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineInputAssemblyStateCreateInfo.html
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkViewport.html
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_ImageExtend.width;
	viewport.height = (float)m_ImageExtend.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkRect2D.html
	VkRect2D scissor{};
	scissor.offset = VkOffset2D{ 0, 0 };
	scissor.extent = m_ImageExtend;

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDynamicState.html
	std::vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineDynamicStateCreateInfo.html
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineViewportStateCreateInfo.html
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineRasterizationStateCreateInfo.html
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineMultisampleStateCreateInfo.html
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineColorBlendAttachmentState.html
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineColorBlendStateCreateInfo.html
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineLayoutCreateInfo.html
	const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo
	{
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,		// sType
		nullptr,											// pNext
		0,													// flags
		1,													// setLayoutCount
		&m_DescriptorSetLayout,								// pSetLayouts
		0,													// pushConstantRangeCount
		nullptr												// pPushConstantRanges
	};

	if (vkCreatePipelineLayout(m_Device, &pipelineLayoutCreateInfo, nullptr, &m_PipeLineLayout) != VK_SUCCESS) throw std::runtime_error("failed to create pipeline layout!");

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkGraphicsPipelineCreateInfo.html
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputStateCreateInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_PipeLineLayout;
	pipelineInfo.renderPass = m_RenderPass;
	pipelineInfo.subpass = 0;

	return vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_PipeLine);
}

VkResult Application::CreateRenderPass()
{
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAttachmentDescription.html
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_ImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAttachmentReference.html
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSubpassDescription.html
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSubpassDependency.html
	VkSubpassDependency subpassDependency{};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkRenderPassCreateInfo.html
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &subpassDependency;

	return vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass);
}

VkResult Application::CreateSwapChainFrameBuffers()
{
	VkResult result{ VK_SUCCESS };

	m_SwapChainFrameBuffers.resize(m_SwapChainImageViews.size());

	for (size_t i{0}; i < m_SwapChainImageViews.size(); ++i)
	{
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFramebufferCreateInfo.html

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = m_RenderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = &m_SwapChainImageViews[i];
		framebufferCreateInfo.width = m_ImageExtend.width;
		framebufferCreateInfo.height = m_ImageExtend.height;
		framebufferCreateInfo.layers = 1;

		result = vkCreateFramebuffer(m_Device, &framebufferCreateInfo, nullptr, &m_SwapChainFrameBuffers[i]);

		if (result != VK_SUCCESS) return result;
	}

	return result;
}

VkResult Application::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices{ FindQueueFamilies(m_PhysicalDevice, m_Surface) };

	VkCommandPoolCreateInfo commandPoolCreateInfo{};

	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();

	return vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_CommandPool);
}

VkResult Application::CreateCommandBuffers()
{
	m_CommandBuffers.resize(g_MaxFramePerFlight);

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandBufferAllocateInfo.html
	VkCommandBufferAllocateInfo allocationInfo{};
	allocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocationInfo.commandPool = m_CommandPool;
	allocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocationInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

	return vkAllocateCommandBuffers(m_Device, &allocationInfo, m_CommandBuffers.data());
}

void Application::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkCommandBufferBeginInfo.html
	VkCommandBufferBeginInfo commandBufferbeginInfo{};
	commandBufferbeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferbeginInfo.flags = 0;
	commandBufferbeginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &commandBufferbeginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin recording the command buffer");
	}

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkRenderPassBeginInfo.html
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_RenderPass;
	renderPassBeginInfo.framebuffer = m_SwapChainFrameBuffers[imageIndex];
	renderPassBeginInfo.renderArea.offset = VkOffset2D{ 0, 0 };
	renderPassBeginInfo.renderArea.extent = m_ImageExtend;
	VkClearValue clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipeLine);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_ImageExtend.width);
	viewport.height = static_cast<float>(m_ImageExtend.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_ImageExtend;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	const VkBuffer vertexBuffers[]{ m_Mesh->GetVertexBuffer() };
	const VkDeviceSize offsets[]{ 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, m_Mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipeLineLayout, 0, 1, &m_DescriptorSets.at(m_CurrentFrame), 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Mesh->GetIndices().size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer");
	}
}

void Application::UpdateUniformBuffer(uint32_t currentImage)
{
	static auto startTime{ std::chrono::high_resolution_clock::now() };
	const auto currentTime{ std::chrono::high_resolution_clock::now() };
	const float time{ std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count() };

	UniformBufferObject matrices
	{
		glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		glm::perspective(glm::radians(45.0f), (float(m_ImageExtend.width) / float(m_ImageExtend.height)), 0.1f, 10.0f)
	};
	matrices.ProjectionMatrix[1][1] *= -1;

	memcpy(m_UniformBufferMaps[currentImage], &matrices, sizeof(UniformBufferObject));
}

void Application::DrawFrame()
{
	vkWaitForFences(m_Device, 1, &m_InFlight[m_CurrentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result{ vkAcquireNextImageKHR(m_Device, m_SwapChain, UINT64_MAX, m_ImageAvailable[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex) };
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire the swap chain image");
	}

	UpdateUniformBuffer(m_CurrentFrame);

	vkResetFences(m_Device, 1, &m_InFlight[m_CurrentFrame]);

	vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);
	RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSubmitInfo.html
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_ImageAvailable[m_CurrentFrame];
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_RenderFinished[m_CurrentFrame];

	if (vkQueueSubmit(m_GrahicsQueue, 1, &submitInfo, m_InFlight[m_CurrentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPresentInfoKHR.html
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_RenderFinished[m_CurrentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_SwapChain;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FrameBufferResized)
	{
		m_FrameBufferResized = false;
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	m_CurrentFrame = (m_CurrentFrame + 1) % g_MaxFramePerFlight;
}

VkResult Application::CreateSyncObjects()
{
	VkResult result{ VK_SUCCESS };

	m_ImageAvailable.resize(g_MaxFramePerFlight);
	m_RenderFinished.resize(g_MaxFramePerFlight);
	m_InFlight.resize(g_MaxFramePerFlight);

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSemaphoreCreateInfo.html
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFenceCreateInfo.html
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i{}; i < g_MaxFramePerFlight; ++i)
	{
		result = vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_ImageAvailable[i]);
		if (result != VK_SUCCESS) return result;

		result = vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_RenderFinished[i]);
		if (result != VK_SUCCESS) return result;

		result = vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &m_InFlight[i]);
		if (result != VK_SUCCESS) return result;
	}

	
	return result;
}

void Application::RecreateSwapChain()
{
	int width{ 0 }, height{ 0 };
	glfwGetFramebufferSize(m_Window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_Window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_Device);

	CleanupSwapChain();

	if (CreateSwapChain() != VK_SUCCESS) throw std::runtime_error("Failed to recreate swap chain");
	RetrieveSwapChainImages();
	if (CreateSwapChainImageViews() != VK_SUCCESS) throw std::runtime_error("Failed to recreate swap chain image views");
	if (CreateSwapChainFrameBuffers() != VK_SUCCESS) throw std::runtime_error("Failed to recreate swap chain frame buffers");
}

void Application::CleanupSwapChain()
{
	for (auto frameBuffer : m_SwapChainFrameBuffers) 
	{
		vkDestroyFramebuffer(m_Device, frameBuffer, nullptr);
	}

	for (auto imageView : m_SwapChainImageViews)
	{
		vkDestroyImageView(m_Device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
}

void Application::FrameBufferResizedCallback(GLFWwindow* window, int width, int height)
{
	width;
	height;

	Application* app{ reinterpret_cast<Application*>(glfwGetWindowUserPointer(window)) };
	app->m_FrameBufferResized = true;
}

VkResult Application::CreateUniformBuffers()
{
	VkResult result{};

	const size_t bufferSize{ sizeof(UniformBufferObject) };

	m_UniformBuffers.resize(g_MaxFramePerFlight);
	m_UniformBufferMemories.resize(g_MaxFramePerFlight);
	m_UniformBufferMaps.resize(g_MaxFramePerFlight);

	for (size_t i{0}; i < g_MaxFramePerFlight; ++i)
	{
		CreateBuffer
		(
			m_PhysicalDevice,
			m_Device,
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_UniformBuffers.at(i),
			m_UniformBufferMemories.at(i)
		);

		result = vkMapMemory(m_Device, m_UniformBufferMemories.at(i), 0, bufferSize, 0, &m_UniformBufferMaps.at(i));
		if (result != VK_SUCCESS) return result;
	}

	return result;
}

VkResult Application::CreateDescriptorPool()
{
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorPoolSize.html
	const VkDescriptorPoolSize descriptorPoolSize
	{
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,				// type
		static_cast<uint32_t>(g_MaxFramePerFlight)		// descriptorCount
	};

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorPoolCreateInfo.html
	const VkDescriptorPoolCreateInfo descriptorPoolCreateInfo
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,	// sType
		nullptr,										// pNext
		0,												// flags
		static_cast<uint32_t>(g_MaxFramePerFlight),		// maxSets
		1,												// poolSizeCount
		&descriptorPoolSize								// pPoolSizes
	};

	return vkCreateDescriptorPool(m_Device, &descriptorPoolCreateInfo, nullptr, &m_DescriptorPool);
}

VkResult Application::CreateDescriptorSets()
{
	m_DescriptorSets.resize(g_MaxFramePerFlight);
	std::vector<VkDescriptorSetLayout> descriptorSetlayouts{ g_MaxFramePerFlight, m_DescriptorSetLayout };

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorSetAllocateInfo.html
	const VkDescriptorSetAllocateInfo descriptorSetAllocateInfo
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,			// sType
		nullptr,												// pNext
		m_DescriptorPool,										// descriptorPool
		static_cast<uint32_t>(g_MaxFramePerFlight),				// descriptorSetCount
		descriptorSetlayouts.data()								// pSetLayouts
	};

	VkResult result{ vkAllocateDescriptorSets(m_Device, &descriptorSetAllocateInfo, m_DescriptorSets.data()) };
	if (result != VK_SUCCESS) return result;

	for (size_t i{ 0 }; i < g_MaxFramePerFlight; i++) 
	{
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorBufferInfo.html
		const VkDescriptorBufferInfo descriptorBufferInfo
		{
			m_UniformBuffers.at(i),				// buffer
			0,									// offset
			sizeof(UniformBufferObject)			// range
		};

		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkWriteDescriptorSet.html
		const VkWriteDescriptorSet writeDescriptorSet
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,			// sType
			nullptr,										// pNext
			m_DescriptorSets.at(i),							// dstSet
			0,												// dstBinding
			0,												// dstArrayElement
			1,												// descriptorCount
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,				// descriptorType
			nullptr,										// pImageInfo
			&descriptorBufferInfo,							// pBufferInfo
			nullptr											// pTexelBufferView
		};

		vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);
	}

	return result;
}