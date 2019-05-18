#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan_win32.h>

#pragma comment(lib, "vulkan-1.lib")

#include <vector>

class AppBase
{
public:

private:
	void InitializeInstance(const char* appName);
	void GetPhysicalDevice();
	uint32_t  SearchGraphicsQueueFamilyIndex();



	void CheckResult(VkResult result);

	VkInstance _instance;
	VkPhysicalDevice _physicalDevice;
	VkPhysicalDeviceMemoryProperties _physicalDeviceMemoryProperties;

};

