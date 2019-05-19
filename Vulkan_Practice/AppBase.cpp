#include "AppBase.h"

// Createなどの結果を受け判定をおこなう
void AppBase::CheckResult(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		DebugBreak();
	}
}


// コンストラクタ
AppBase::AppBase() : _presentMode(VK_PRESENT_MODE_FIFO_KHR), _imageIndex(0)
{
}



// アプリケーションの初期化をおこなう
void AppBase::Initialize(GLFWwindow* window, const char* appName)
{
	// インスタンスの生成
	InitializeInstance(appName);

	// 物理デバイスの取得
	GetPhysicalDevice();

	// グラフィックス用のキューファミリーインデックスの取得
	_graphicsQueueFamilyIndex = SearchGraphicsQueueFamilyIndex();

	// 論理デバイスの生成
	CreateDevice();

	// コマンドプールの作成
	CreateCommandPool();

	// Surface生成
	glfwCreateWindowSurface(_instance, window, nullptr, &_surface);

	// Surfaceのフォーマット選択
	SelectSurfaceFormat(VK_FORMAT_B8G8R8A8_UNORM);

	// Surfaceの能力値取得
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &_surfaceCapabilities);

	// Swapchainがサポートされているか確認
	VkBool32 isSupported;
	vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, _graphicsQueueFamilyIndex, _surface, &isSupported);

}


// Instanceを生成する
void AppBase::InitializeInstance(const char* appName)
{
	// ApplicationInfoの初期化
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = appName;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = appName;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	// Instance extensionの取得
	std::vector<const char*> extensions;
	std::vector<VkExtensionProperties> props;
	{
		uint32_t count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
		props.resize(count);
		vkEnumerateInstanceExtensionProperties(nullptr, &count, props.data());

		for (const auto& v : props)
		{
			extensions.push_back(v.extensionName);
		}
	}

	// Instance Create Infoの初期化
	VkInstanceCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ci.pNext = nullptr;
	ci.flags = 0;
	ci.pApplicationInfo = &appInfo;

#ifdef _DEBUG
	// Debug時は下記のmeta layerを有効にする
	const char* layers[] = { "VK_LAYER_LUNARG_standard_validation" };
	ci.enabledLayerCount = 1;
	ci.ppEnabledLayerNames = layers;
#endif

	ci.enabledExtensionCount = uint32_t(extensions.size());
	ci.ppEnabledExtensionNames = extensions.data();

	// Instanceの生成
	auto result = vkCreateInstance(&ci, nullptr, &_instance);
	CheckResult(result);
}


// 物理デバイスを取得する
void AppBase::GetPhysicalDevice()
{
	// 接続されている物理デバイスを列挙する
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(_instance, &count, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices(count);
	vkEnumeratePhysicalDevices(_instance, &count, physicalDevices.data());

	// 最初のデバイスのメモリプロパティを取得する
	_physicalDevice = physicalDevices[0];
	vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_physicalDeviceMemoryProperties);
}


// グラフィックス用のキューファミリーインデックスを取得する
uint32_t AppBase::SearchGraphicsQueueFamilyIndex()
{
	// 物理デバイスのキューファミリープロパティを取得する
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &count, nullptr);
	std::vector<VkQueueFamilyProperties> props(count);
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &count, props.data());

	// 取得したキューファミリーのうち、グラフィックス用のキューファミリーのインデックスを取得する
	uint32_t graphicsQueue = ~0u;
	for (uint32_t i = 0; i < count; ++i)
	{
		if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsQueue = i;
			break;
		}
	}

	return graphicsQueue;
}	


// 論理デバイスを作成する
void AppBase::CreateDevice()
{
	const float defaultQueuePriority(1.0f);
	VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.queueFamilyIndex = _graphicsQueueFamilyIndex;
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.pQueuePriorities = &defaultQueuePriority;

	// device extensionの取得
	std::vector<const char*> extensions;
	std::vector<VkExtensionProperties> props;
	{
		uint32_t count = 0;
		vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &count, nullptr);
		props.resize(count);
		vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &count, props.data());

		for (const auto& v : props)
		{
			extensions.push_back(v.extensionName);
		}
	}

	// Device Create Info の初期化
	VkDeviceCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	ci.pQueueCreateInfos = &deviceQueueCreateInfo;
	ci.queueCreateInfoCount = 1;
	ci.ppEnabledExtensionNames = extensions.data();
	ci.enabledExtensionCount = uint32_t(extensions.size());


	// デバイスの生成
	auto result = vkCreateDevice(_physicalDevice, &ci, nullptr, &_device);
	CheckResult(result);

	// デバイスキューの取得
	vkGetDeviceQueue(_device, _graphicsQueueFamilyIndex, 0, &_deviceQueue);
}


// コマンドプールの作成
void AppBase::CreateCommandPool()
{
	VkCommandPoolCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	ci.queueFamilyIndex = _graphicsQueueFamilyIndex;
	ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	auto result = vkCreateCommandPool(_device, &ci, nullptr, &_commandPool);
	CheckResult(result);
}


// Surfaceのフォーマットを選択する
void AppBase::SelectSurfaceFormat(VkFormat format)
{
	uint32_t count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &count, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &count, formats.data());
	
	for (const auto& f : formats)
	{
		if (f.format == format)
		{
			_surfaceFormat = f;
		}
	}
}


// Swapchainを生成する
void AppBase::CreateSwapchain(GLFWwindow* window)
{
	auto minImageCount = (std::max)(2u, _surfaceCapabilities.minImageCount);
	auto extent = _surfaceCapabilities.currentExtent;
	if (extent.width == ~0u)
	{
		// 無効な値の場合、ウィンドウサイズを使用する
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		extent.width = uint32_t(width);
		extent.height = uint32_t(height);
	}

	uint32_t queueFamilyIndices[] = { _graphicsQueueFamilyIndex };

	VkSwapchainCreateInfoKHR ci{};
	ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	ci.surface = _surface;
	ci.minImageCount = minImageCount;
	ci.imageFormat = _surfaceFormat.format;
	ci.imageColorSpace = _surfaceFormat.colorSpace;
	ci.imageExtent = extent;
	ci.imageArrayLayers = 1;
	ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ci.queueFamilyIndexCount = 0;
	ci.preTransform = _surfaceCapabilities.currentTransform;
	ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	ci.presentMode = _presentMode;
	ci.clipped = VK_TRUE;
	ci.oldSwapchain = VK_NULL_HANDLE;

	auto result = vkCreateSwapchainKHR(_device, &ci, nullptr, &_swapchain);
	CheckResult(result);
	_swapchainExtent2D = extent;
}