#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

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
#include "Texture.h"
#include "Camera.h"

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
	m_Meshes{},
	m_UniformBuffers{},
	m_UniformBufferMemories{},
	m_UniformBufferMaps{},
	m_DescriptorPool{},
	m_DescriptorSets{},
	m_BaseColorTexture{},
	m_TextureSampler{},
	m_DepthImage{},
	m_DepthMemory{},
	m_DepthImageView{},
	m_Camera{}
{
	InitializeWindow();
	InitializeVulkan();
	InitializeMeshes();

	m_Camera = new Camera{ glm::radians(45.0f), (float(m_ImageExtend.width) / float(m_ImageExtend.height)), 0.1f, 10.0f, 2.5f };
	m_Camera->SetStartPosition(glm::vec3{ 2.83f, 2.09f, 1.41f }, 0.63f, -0.39f);

	//m_Camera = new Camera{ glm::radians(45.0f), (float(m_ImageExtend.width) / float(m_ImageExtend.height)), 0.1f, 1000.0f, 15.0f };
	//m_Camera->SetStartPosition(glm::vec3{ 34.68f, 29.46f, 10.52f }, 0.73f, -0.29f);
}

Application::~Application()
{
	delete m_Camera;
	vkDestroySampler(m_Device, m_TextureSampler, nullptr);
	delete m_BaseColorTexture;
	for (auto mesh : m_Meshes)
	{
		delete mesh;
	}
	for (int i{}; i < g_MaxFramePerFlight; ++i)
	{
		vkDestroyBuffer(m_Device, m_UniformBuffers.at(i), nullptr);
		vkFreeMemory(m_Device, m_UniformBufferMemories.at(i), nullptr);
	}
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
	glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	auto lastTime{ std::chrono::high_resolution_clock::now() };
	auto currentTime{ std::chrono::high_resolution_clock::now() };

	while (!glfwWindowShouldClose(m_Window))
	{
		currentTime = std::chrono::high_resolution_clock::now();
		auto time{ std::chrono::duration_cast<std::chrono::duration<float>>(currentTime - lastTime) };

		glfwPollEvents();
		m_Camera->Update(m_Window, time);
		DrawFrame();

		lastTime = currentTime;
	}

	vkDeviceWaitIdle(m_Device);
}

void Application::InitializeMeshes()
{
	m_Meshes.push_back(new Mesh{m_PhysicalDevice, m_Device, m_CommandPool, m_GrahicsQueue, "Models/viking_room.obj"});

	//m_Meshes.push_back(new Mesh{ m_PhysicalDevice, m_Device, m_CommandPool, m_GrahicsQueue, "Models/vehicle.obj" });
	//m_Meshes.at(0)->SetModelMatrix(glm::rotate(glm::mat4{ 1.0f }, glm::radians(-90.0f), g_WorldForward));
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
	if (CreateCommandPool() != VK_SUCCESS) throw std::runtime_error("failed to create command pool!");
	CreateDepthResources();
	if (CreateSwapChainFrameBuffers() != VK_SUCCESS) throw std::runtime_error("failed to create swap chain frame buffers!");
	if (CreateCommandBuffers() != VK_SUCCESS) throw std::runtime_error("failed to create command buffer!");
	if (CreateSyncObjects() != VK_SUCCESS) throw std::runtime_error("failed to create sync objects!");
	m_BaseColorTexture = new Texture{ m_PhysicalDevice, m_Device, m_CommandPool, m_GrahicsQueue, "Textures/viking_room.png" };
	//m_BaseColorTexture = new Texture{ m_PhysicalDevice, m_Device, m_CommandPool, m_GrahicsQueue, "Textures/vehicle_diffuse.png" };
	CreateTextureSampler();
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
	physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

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
		m_SwapChainImageViews.at(i) = CreateImageView(m_Device, m_SwapChainImages.at(i), m_ImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	return VK_SUCCESS;
}

VkResult Application::CreateDescriptorSetLayout()
{
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorSetLayoutBinding.html
	const std::array<VkDescriptorSetLayoutBinding, 2> descriptorSetLayoutBindings
	{
		// Uniform buffer object
		VkDescriptorSetLayoutBinding
		{
			0,											// binding
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			// descriptorType
			1,											// descriptorCount
			VK_SHADER_STAGE_VERTEX_BIT,					// stageFlags
			nullptr										// pImmutableSamplers
		},
		// texture + sampler
		VkDescriptorSetLayoutBinding
		{
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			nullptr
		}
	};

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorSetLayoutCreateInfo.html
	const VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,		// sType
		nullptr,													// pNext
		0,															// flags
		uint32_t(descriptorSetLayoutBindings.size()),				// bindingCount
		descriptorSetLayoutBindings.data()							// pBindings
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
	std::vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

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

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineDepthStencilStateCreateInfo.html
	const VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilCreateInfo
	{
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,				// sType
		nullptr,																// pNext
		0,																		// flags
		VK_TRUE,																// depthTestEnable
		VK_TRUE,																// depthWriteEnable
		VK_COMPARE_OP_LESS,														// depthCompareOp
		VK_FALSE,																// depthBoundsTestEnable
		VK_FALSE,																// stencilTestEnable
		VkStencilOpState{},														// front
		VkStencilOpState{},														// back
		0.0f,																	// minDepthBounds
		1.0f																	// maxDepthBounds
	};

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
	const VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo
	{
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,		// sType
		nullptr,												// pNext
		0,														// flags
		2,														// stageCount
		shaderStages,											// pStages
		&vertexInputStateCreateInfo,							// pVertexInputState
		&inputAssembly,											// pInputAssemblyState
		nullptr,												// pTessellationState
		&viewportState,											// pViewportState
		&rasterizer,											// pRasterizationState
		&multisampling,											// pMultisampleState
		&pipelineDepthStencilCreateInfo,						// pDepthStencilState
		&colorBlending,											// pColorBlendState
		&dynamicState,											// pDynamicState
		m_PipeLineLayout,										// layout
		m_RenderPass,											// renderPass
		0,														// subpass
		nullptr,												// basePipelineHandle
		0														// basePipelineIndex
	};

	return vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &m_PipeLine);
}

VkResult Application::CreateRenderPass()
{
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAttachmentDescription.html
	std::array<VkAttachmentDescription, 2> attachmentDescriptions
	{
		// color attachment
		VkAttachmentDescription
		{
			0,													// flags
			m_ImageFormat,										// format
			VK_SAMPLE_COUNT_1_BIT,								// samples
			VK_ATTACHMENT_LOAD_OP_CLEAR,						// loadOp
			VK_ATTACHMENT_STORE_OP_STORE,						// storeOp
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,					// stencilLoadOp
			VK_ATTACHMENT_STORE_OP_DONT_CARE,					// stencilStoreOp
			VK_IMAGE_LAYOUT_UNDEFINED,							// initialLayout
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR						// finalLayout
		},
		// depth buffering attachment
		VkAttachmentDescription
		{
			0,
			FindDepthFormat(m_PhysicalDevice),
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		},
	};

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAttachmentReference.html
	const VkAttachmentReference depthAttachmentReference
	{
		1,														// attachment
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL		// layout	
	};

	const VkAttachmentReference colorAttachmentReference
	{
		0,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSubpassDescription.html
	const VkSubpassDescription subpassDescription
	{
		0,											// flags
		VK_PIPELINE_BIND_POINT_GRAPHICS,			// pipelineBindPoint
		0,											// inputAttachmentCount
		nullptr,									// pInputAttachments
		1,											// colorAttachmentCount
		&colorAttachmentReference,					// pColorAttachments
		nullptr,									// pResolveAttachments
		&depthAttachmentReference,					// pDepthStencilAttachment
		0,											// preserveAttachmentCount
		nullptr										// pPreserveAttachments
	};

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSubpassDependency.html
	const VkSubpassDependency subpassDependency
	{
		VK_SUBPASS_EXTERNAL,																			// srcSubpass
		0,																								// dstSubpass
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,		// srcStageMask	
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,		// dstStageMask	
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,													// srcAccessMask	
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,			// dstAccessMask
		0																								// dependencyFlags
	};

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkRenderPassCreateInfo.html
	const VkRenderPassCreateInfo renderPassCreateInfo
	{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,			// sType
		nullptr,											// pNext
		0,													// flags
		uint32_t(attachmentDescriptions.size()),			// attachmentCount
		attachmentDescriptions.data(),						// pAttachments
		1,													// subpassCount
		&subpassDescription,								// pSubpasses
		1,													// dependencyCount
		&subpassDependency									// pDependencies
	};

	return vkCreateRenderPass(m_Device, &renderPassCreateInfo, nullptr, &m_RenderPass);
}

VkResult Application::CreateSwapChainFrameBuffers()
{
	VkResult result{ VK_SUCCESS };

	m_SwapChainFrameBuffers.resize(m_SwapChainImageViews.size());

	for (size_t i{ 0 }; i < m_SwapChainImageViews.size(); ++i)
	{
		const std::array<VkImageView, 2> attachments{ m_SwapChainImageViews.at(i), m_DepthImageView };

		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFramebufferCreateInfo.html
		const VkFramebufferCreateInfo frameBufferCreateInfo
		{
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,				// sType
			nullptr,												// pNext
			0,														// flags
			m_RenderPass,											// renderPass
			uint32_t(attachments.size()),							// AttachmentCount
			attachments.data(),										// pAttachments
			m_ImageExtend.width,									// width
			m_ImageExtend.height,									// height
			1														// layers
		};

		result = vkCreateFramebuffer(m_Device, &frameBufferCreateInfo, nullptr, &m_SwapChainFrameBuffers.at(i));
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
	const VkCommandBufferBeginInfo commandBufferBeginInfo
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,			// sType
		nullptr,												// pNext
		0,														// flags
		nullptr													// pInheritanceInfo
	};

	if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin recording the command buffer");
	}

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkClearValue.html
	std::array<VkClearValue, 2> clearColors
	{
		VkClearValue{ 0.39f, 0.59f, 0.93f, 1.0f },
		VkClearValue{ 1.0f, 1 }
	};

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkRenderPassBeginInfo.html
	const VkRenderPassBeginInfo renderPassBeginInfo
	{
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,													// sType
		nullptr,																					// pNext
		m_RenderPass,																				// renderPass
		m_SwapChainFrameBuffers[imageIndex],														// framebuffer
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkRect2D.html
		VkRect2D																					// renderArea
		{
			VkOffset2D{ 0, 0 },		// offset
			m_ImageExtend			// extent
		},
		uint32_t(clearColors.size()),																// clearValueCount
		clearColors.data()																			// pClearValues
	};

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

	const VkBuffer vertexBuffers[]{ m_Meshes.at(0)->GetVertexBuffer() };
	const VkDeviceSize offsets[]{ 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, m_Meshes.at(0)->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipeLineLayout, 0, 1, &m_DescriptorSets.at(m_CurrentFrame), 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Meshes.at(0)->GetIndices().size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer");
	}
}

void Application::UpdateUniformBuffer(uint32_t currentImage, Mesh* mesh)
{
	UniformBufferObject matrices
	{
		mesh->GetModelMatrix(),
		m_Camera->GetViewMatrx(),
		m_Camera->GetProjectionMatrix()
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

	UpdateUniformBuffer(m_CurrentFrame, m_Meshes.at(0));

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
	CreateDepthResources();
	if (CreateSwapChainFrameBuffers() != VK_SUCCESS) throw std::runtime_error("Failed to recreate swap chain frame buffers");
}

void Application::CleanupSwapChain()
{
	vkDestroyImageView(m_Device, m_DepthImageView, nullptr);
	vkDestroyImage(m_Device, m_DepthImage, nullptr);
	vkFreeMemory(m_Device, m_DepthMemory, nullptr);

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

	for (size_t i{ 0 }; i < g_MaxFramePerFlight; ++i)
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
	std::array< VkDescriptorPoolSize, 2> descriptorPoolSizes
	{
		VkDescriptorPoolSize
		{
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,				// type
			static_cast<uint32_t>(g_MaxFramePerFlight)		// descriptorCount
		},
		VkDescriptorPoolSize
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			static_cast<uint32_t>(g_MaxFramePerFlight)
		}
	};

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorPoolCreateInfo.html
	const VkDescriptorPoolCreateInfo descriptorPoolCreateInfo
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,					// sType
		nullptr,														// pNext
		0,																// flags
		static_cast<uint32_t>(g_MaxFramePerFlight),						// maxSets
		static_cast<uint32_t>(descriptorPoolSizes.size()),				// poolSizeCount
		descriptorPoolSizes.data()										// pPoolSizes
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

		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorImageInfo.html
		const VkDescriptorImageInfo descriptorImageInfo
		{
			m_TextureSampler,								// sampler
			m_BaseColorTexture->GetImageView(),				// imageView
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL		// imageLayout
		};

		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkWriteDescriptorSet.html
		std::array<VkWriteDescriptorSet, 2> writeDescriptorSets
		{
			// Uniform buffer object
			VkWriteDescriptorSet
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
			},
			// Texture + sampler
			VkWriteDescriptorSet
			{
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				nullptr,
				m_DescriptorSets.at(i),
				1,
				0,
				1,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				&descriptorImageInfo,
				nullptr,
				nullptr
			},
		};

		vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}

	return result;
}

void Application::CreateTextureSampler()
{
	VkPhysicalDeviceProperties physicalDeviceProperties{};
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &physicalDeviceProperties);

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSamplerCreateInfo.html
	const VkSamplerCreateInfo samplerCreateInfo
	{
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,							// sType
		nullptr,														// pNext
		0,																// flags
		VK_FILTER_LINEAR,												// magFilter
		VK_FILTER_LINEAR,												// minFilter
		VK_SAMPLER_MIPMAP_MODE_LINEAR,									// mipmapMode	
		VK_SAMPLER_ADDRESS_MODE_REPEAT,									// addressModeU	
		VK_SAMPLER_ADDRESS_MODE_REPEAT,									// addressModeV	
		VK_SAMPLER_ADDRESS_MODE_REPEAT,									// addressModeW	
		0.0f,															// mipLodBias
		VK_TRUE,														// anisotropyEnable
		physicalDeviceProperties.limits.maxSamplerAnisotropy,			// maxAnisotropy
		VK_FALSE,														// compareEnable
		VK_COMPARE_OP_ALWAYS,											// compareOp	
		0.0f,															// minLod
		VK_LOD_CLAMP_NONE,												// maxLod
		VK_BORDER_COLOR_INT_OPAQUE_BLACK,								// borderColor
		VK_FALSE														// unnormalizedCoordinates
	};

	if (vkCreateSampler(m_Device, &samplerCreateInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sampler!");
	}
}

void Application::CreateDepthResources()
{
	VkFormat depthFormat{ FindDepthFormat(m_PhysicalDevice) };

	CreateImage
	(
		m_PhysicalDevice,
		m_Device,
		m_ImageExtend,
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_DepthImage,
		m_DepthMemory,
		1
	);

	m_DepthImageView = CreateImageView(m_Device, m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	TransitionImageLayout(m_Device, m_CommandPool, m_GrahicsQueue, m_DepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}