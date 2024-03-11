#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <format>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "Application.h"

Application::Application(int width, int height) :
	m_Width{ width },
	m_Height{ height },
	m_Window{ nullptr }
{
	InitializeWindow();
	InitializeVulkan();
}

Application::~Application()
{
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
	if(m_Window == nullptr) throw std::runtime_error("failed to create window!");
}

void Application::InitializeVulkan()
{
	if (GLFWExtensionsPresent())
	{
		if (CreateVulkanInstance() != VK_SUCCESS) throw std::runtime_error("failed to create vulkan instance!");
	}
	else throw std::runtime_error("vulkan doesn't have the required extensions for glfw!");
}

bool Application::GLFWExtensionsPresent()
{
	uint32_t vulkanExtensionCount{};
	vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, nullptr);
	std::vector<VkExtensionProperties> vulkanExtensions(vulkanExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &vulkanExtensionCount, vulkanExtensions.data());
	std::vector<std::string> vulkanExtensionNames{};
	for (const auto& extenstion : vulkanExtensions)
	{
		vulkanExtensionNames.push_back(std::string{ extenstion.extensionName });
	}

	uint32_t glfwExtensionCount{};
	const char** glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };
	std::vector<std::string> glfwExtensionNames{ glfwExtensionCount };
	for (uint32_t i{}; i < glfwExtensionCount; ++i)
	{
		std::stringstream stringStream{};
		stringStream << glfwExtensions[i];
		glfwExtensionNames.at(i) = stringStream.str();
	}

	bool allRequiredExtensionsPresent{ true };
	std::cout << std::endl << "-----GLFW required extensions-----" << std::endl;
	for (const auto& extensionName : glfwExtensionNames)
	{
		std::cout << std::setw(40) << std::left << extensionName;

		if (std::ranges::find(vulkanExtensionNames, extensionName) != vulkanExtensionNames.end())
		{
			std::cout << std::setw(40) << std::left << "PRESENT";
		}
		else
		{
			allRequiredExtensionsPresent = false;
			std::cout << std::setw(40) << std::left << "NOT PRESENT";
		}

		std::cout << std::endl;
	}
	std::cout << std::endl;

	return allRequiredExtensionsPresent;
}

VkResult Application::CreateVulkanInstance()
{
	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkApplicationInfo.html
	VkApplicationInfo applicationInformation{};
	applicationInformation.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInformation.pApplicationName = "Hello Triangle";
	applicationInformation.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInformation.pEngineName = "No Engine";
	applicationInformation.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInformation.apiVersion = VK_API_VERSION_1_3;

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkInstanceCreateInfo.html
	VkInstanceCreateInfo createInformation{};
	createInformation.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInformation.pApplicationInfo = &applicationInformation;
	uint32_t glfwExtensionCount{};
	createInformation.ppEnabledExtensionNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	createInformation.enabledExtensionCount = glfwExtensionCount;
	createInformation.enabledLayerCount = 0;

	return vkCreateInstance(&createInformation, nullptr, &m_Instance);
}