#define GLFW_INCLUDE_VULKAN

#ifdef NDEBUG
const bool g_EnableValidationlayers{ false };
#else
const bool g_EnableValidationlayers{ true };
#endif

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <cstring>
#include <set>

#include "Application.h"
#include "HelperFunctions.h"

Application::Application(int width, int height) :
	m_Width{ width },
	m_Height{ height },
	m_Window{ nullptr },
	m_Instance{},
	m_DebugMessenger{},
	m_Surface{},
	m_ValidationLayerNames{ "VK_LAYER_KHRONOS_validation" },
	m_ExtensionNames{},
	m_PhysicalDevice{ VK_NULL_HANDLE },
	m_Device{ VK_NULL_HANDLE },
	m_GrahicsQueue{ VK_NULL_HANDLE },
	m_PresentQueue{ VK_NULL_HANDLE }
{
	InitializeWindow();
	InitializeVulkan();
}

Application::~Application()
{
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
	}
}

void Application::InitializeWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_Window = glfwCreateWindow(m_Width, m_Height, "Vulkan", nullptr, nullptr);
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
}

bool Application::ExtensionsPresent()
{
	uint32_t vulkanExtensionCount{};
	vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(vulkanExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, extensions.data());

	uint32_t glfwExtensionCount{};
	const char** glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };
	m_ExtensionNames = std::vector<const char*>{ glfwExtensions, glfwExtensions + glfwExtensionCount };
	if (g_EnableValidationlayers) m_ExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	bool extensionsPresent{ true };
	std::cout << std::endl << "-----Extensions-----" << std::endl;
	for (const auto& extensionName : m_ExtensionNames)
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
	std::cout << std::endl << "-----Validation layers-----" << std::endl;
	for (const auto& validationLayerName : m_ValidationLayerNames)
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
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_ExtensionNames.size());
	instanceCreateInfo.ppEnabledExtensionNames = m_ExtensionNames.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (g_EnableValidationlayers)
	{
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayerNames.size());
		instanceCreateInfo.ppEnabledLayerNames = m_ValidationLayerNames.data();

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
		if (IsPhysicalDeviceSuitable(device, m_Surface))
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
	deviceCreateInfo.pQueueCreateInfos = queueFamailyCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueFamailyCreateInfos.size());
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
	deviceCreateInfo.enabledExtensionCount = 0;
	if (g_EnableValidationlayers)
	{
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayerNames.size());
		deviceCreateInfo.ppEnabledLayerNames = m_ValidationLayerNames.data();
	}
	else
	{
		deviceCreateInfo.enabledLayerCount = 0;
	}
	
	return vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
}

void Application::RetrieveQueueHandles()
{
	QueueFamilyIndices queueFamilyIndices{ FindQueueFamilies(m_PhysicalDevice, m_Surface) };

	vkGetDeviceQueue(m_Device, queueFamilyIndices.GraphicsFamily.value(), 0, &m_GrahicsQueue);
	vkGetDeviceQueue(m_Device, queueFamilyIndices.PresentFamily.value(), 0, &m_PresentQueue);
}