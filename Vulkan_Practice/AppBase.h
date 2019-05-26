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
#include <algorithm>
#include <array>
#include <sstream>

class AppBase
{
public:
	AppBase();
	virtual ~AppBase() {}
	void Initialize(GLFWwindow* window, const char* appName);
	void Terminate();

private:

	void InitializeInstance(const char* appName);
	void GetPhysicalDevice();
	uint32_t  SearchGraphicsQueueFamilyIndex();
	void CreateDevice();
	void CreateCommandPool();
	void SelectSurfaceFormat(VkFormat format);
	void CreateSwapchain(GLFWwindow* window);
	void CreateDepthBuffer();
	uint32_t GetMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestProps)const;

	void CreateImageViews();

	void CreateRenderPass();
	void CreateFramebuffer();

	void AllocateCommandBuffers();
	void CreateFence();


	void CheckResult(VkResult result);
	
	void EnableDebugReport();


	VkInstance _instance;
	VkDevice _device;

	VkPhysicalDevice _physicalDevice;
	VkPhysicalDeviceMemoryProperties _physicalDeviceMemoryProperties;

	VkSurfaceKHR _surface;
	VkSurfaceFormatKHR _surfaceFormat;
	VkSurfaceCapabilitiesKHR _surfaceCapabilities;

	uint32_t _graphicsQueueFamilyIndex;
	VkQueue _deviceQueue;

	VkCommandPool _commandPool;
	VkPresentModeKHR _presentMode;
	VkSwapchainKHR _swapchain;
	VkExtent2D _swapchainExtent2D;
	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageViews;

	VkImage _depthBuffer;
	VkImageView _depthBufferView;
	VkDeviceMemory _depthBufferMemory;

	VkRenderPass _renderPass;
	std::vector<VkFramebuffer> _framebuffers;

	std::vector<VkFence> _fences;
	VkSemaphore _renderCompletedSemaphore;
	VkSemaphore _presentCompletedSemaphore;


	// デバッグレポート用
	PFN_vkCreateDebugReportCallbackEXT _createDebugReportCallback;
	PFN_vkDebugReportMessageEXT _debugReportMessage;
	PFN_vkDestroyDebugReportCallbackEXT _destroyDebugReportCallback;
	VkDebugReportCallbackEXT _debugReportCallback;

	std::vector<VkCommandBuffer> _commandBuffers;



	uint32_t  _imageIndex;

};

