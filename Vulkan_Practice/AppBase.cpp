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

#ifdef _DEBUG
	// デバッグレポート関数有効化
	EnableDebugReport();
#endif

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

	// Swapchain生成
	CreateSwapchain(window);

	// depth buffer 生成
	CreateDepthBuffer();

	// 各ImageViewの生成
	CreateImageViews();

	// レンダーパスの生成
	CreateRenderPass();

	CreateFramebuffer();

	AllocateCommandBuffers();

	CreateFence();

	CreateSemaphores();
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
			extensions.emplace_back(v.extensionName);
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
			extensions.emplace_back(v.extensionName);
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

// Depth Bufferを生成する
void AppBase::CreateDepthBuffer()
{
	VkImageCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ci.imageType = VK_IMAGE_TYPE_2D;
	ci.format = VK_FORMAT_D32_SFLOAT;
	ci.extent.width = _swapchainExtent2D.width;
	ci.extent.height = _swapchainExtent2D.height;
	ci.extent.depth = 1;
	ci.mipLevels = 1;
	ci.arrayLayers = 1;
	ci.samples = VK_SAMPLE_COUNT_1_BIT;
	ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	// VkImage の生成
	auto result = vkCreateImage(_device, &ci, nullptr, &_depthBuffer);
	CheckResult(result);

	// メモリの割当とバインド
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(_device, _depthBuffer, &memoryRequirements);
	VkMemoryAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.allocationSize = memoryRequirements.size;
	ai.memoryTypeIndex = GetMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(_device, &ai, nullptr, &_depthBufferMemory);
	vkBindImageMemory(_device, _depthBuffer, _depthBufferMemory, 0);
}

// メモリタイプの取得
uint32_t AppBase::GetMemoryTypeIndex(uint32_t requestBits, VkMemoryPropertyFlags requestProps)const
{
	uint32_t result = ~0u;
	for (uint32_t i = 0; i < _physicalDeviceMemoryProperties.memoryTypeCount; ++i)
	{
		if (requestBits & 1)
		{
			const auto& types = _physicalDeviceMemoryProperties.memoryTypes[i];
			if ((types.propertyFlags & requestProps) == requestProps)
			{
				result = i;
				break;
			}
		}
		requestBits >>= 1;
	}
	return result;
}

void AppBase::CreateImageViews()
{
	// swapchain
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr);
	_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, _swapchainImages.data());
	_swapchainImageViews.resize(imageCount);
	
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		VkImageViewCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ci.format = _surfaceFormat.format;
		ci.components = {
		  VK_COMPONENT_SWIZZLE_R,
		  VK_COMPONENT_SWIZZLE_G,
		  VK_COMPONENT_SWIZZLE_B,
		  VK_COMPONENT_SWIZZLE_A,
		};
		ci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		ci.image = _swapchainImages[i];
		auto result = vkCreateImageView(_device, &ci, nullptr, &_swapchainImageViews[i]);
		CheckResult(result);
	}

	// depthbuffer
	{
		VkImageViewCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ci.format = VK_FORMAT_D32_SFLOAT;
		ci.components = {
		  VK_COMPONENT_SWIZZLE_R,
		  VK_COMPONENT_SWIZZLE_G,
		  VK_COMPONENT_SWIZZLE_B,
		  VK_COMPONENT_SWIZZLE_A,
		};
		ci.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
		ci.image = _depthBuffer;
		auto result = vkCreateImageView(_device, &ci, nullptr, &_depthBufferView);
		CheckResult(result);
	}
}

void AppBase::CreateRenderPass()
{
	// attachment descriptions
	std::array<VkAttachmentDescription, 2> attachmentDescriptions;
	auto& colorTarget = attachmentDescriptions[0];
	auto& depthTarget = attachmentDescriptions[1];

	colorTarget = VkAttachmentDescription{};
	colorTarget.format = _surfaceFormat.format;
	colorTarget.samples = VK_SAMPLE_COUNT_1_BIT;
	colorTarget.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorTarget.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorTarget.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorTarget.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorTarget.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorTarget.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	depthTarget = VkAttachmentDescription{};
	depthTarget.format = VK_FORMAT_D32_SFLOAT;
	depthTarget.samples = VK_SAMPLE_COUNT_1_BIT;
	depthTarget.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthTarget.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthTarget.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthTarget.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthTarget.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthTarget.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// attachment references
	VkAttachmentReference colorReference{}, depthReference{};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// subpass description
	VkSubpassDescription subpassDesc{};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.colorAttachmentCount = 1;
	subpassDesc.pColorAttachments = &colorReference;
	subpassDesc.pDepthStencilAttachment = &depthReference;

	// create render pass 
	VkRenderPassCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	ci.attachmentCount = uint32_t(attachmentDescriptions.size());
	ci.pAttachments = attachmentDescriptions.data();
	ci.subpassCount = 1;
	ci.pSubpasses = &subpassDesc;

	auto result = vkCreateRenderPass(_device, &ci, nullptr, &_renderPass);
	CheckResult(result);
}

void AppBase::CreateFramebuffer()
{
	VkFramebufferCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	ci.renderPass = _renderPass;
	ci.width = _swapchainExtent2D.width;
	ci.height = _swapchainExtent2D.height;
	ci.layers = 1;

	_framebuffers.clear();
	for (auto& v : _swapchainImageViews)
	{
		std::array<VkImageView, 2> imageViews;
		ci.attachmentCount = uint32_t(imageViews.size());
		ci.pAttachments = imageViews.data();
		imageViews[0] = v;
		imageViews[1] = _depthBufferView;

		VkFramebuffer framebuffer;
		auto result = vkCreateFramebuffer(_device, &ci, nullptr, &framebuffer);
		CheckResult(result);
		_framebuffers.emplace_back(framebuffer);
	}
}

void AppBase::AllocateCommandBuffers()
{
	VkCommandBufferAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	ai.commandPool = _commandPool;
	ai.commandBufferCount = uint32_t(_swapchainImageViews.size());
	ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	_commandBuffers.resize(ai.commandBufferCount);
	auto result = vkAllocateCommandBuffers(_device, &ai, _commandBuffers.data());
	CheckResult(result);
}

void AppBase::CreateFence()
{
	VkFenceCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	_fences.resize(_commandBuffers.size());
	for (auto& v : _fences)
	{
		auto result = vkCreateFence(_device, &ci, nullptr, &v);
		CheckResult(result);
	}
}

void AppBase::CreateSemaphores()
{
	VkSemaphoreCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(_device, &ci, nullptr, &_renderCompletedSemaphore);
	vkCreateSemaphore(_device, &ci, nullptr, &_presentCompletedSemaphore);
}

void AppBase::Render()
{
	uint32_t nextImageIndex = 0;
	vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX, _presentCompletedSemaphore, VK_NULL_HANDLE, &nextImageIndex);
	auto commandFence = _fences[nextImageIndex];
	vkWaitForFences(_device, 1, &commandFence, VK_TRUE, UINT64_MAX);

	// クリア値
	std::array<VkClearValue, 2> clearValue = {
	  { {0.5f, 0.25f, 0.25f, 0.0f}, // Clear color
		{1.0f, 0 } // Clear Depth
	  }
	};

	VkRenderPassBeginInfo renderPassBI{};
	renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBI.renderPass = _renderPass;
	renderPassBI.framebuffer = _framebuffers[nextImageIndex];
	renderPassBI.renderArea.offset = VkOffset2D{ 0, 0 };
	renderPassBI.renderArea.extent = _swapchainExtent2D;
	renderPassBI.pClearValues = clearValue.data();
	renderPassBI.clearValueCount = uint32_t(clearValue.size());

	// コマンドバッファ・レンダーパス開始
	VkCommandBufferBeginInfo commandBI{};
	commandBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	auto& command = _commandBuffers[nextImageIndex];

	vkBeginCommandBuffer(command, &commandBI);
	vkCmdBeginRenderPass(command, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

	_imageIndex = nextImageIndex;
	CreateCommand(command);

	// コマンド・レンダーパス終了
	vkCmdEndRenderPass(command);
	vkEndCommandBuffer(command);

	// コマンドを実行（送信)
	VkSubmitInfo submitInfo{};
	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &command;
	submitInfo.pWaitDstStageMask = &waitStageMask;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &_presentCompletedSemaphore;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &_renderCompletedSemaphore;
	vkResetFences(_device, 1, &commandFence);
	vkQueueSubmit(_deviceQueue, 1, &submitInfo, commandFence);

	// Present 処理
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_swapchain;
	presentInfo.pImageIndices = &nextImageIndex;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &_renderCompletedSemaphore;
	vkQueuePresentKHR(_deviceQueue, &presentInfo);
}

void AppBase::Terminate()
{
	vkDeviceWaitIdle(_device);

	Clean();

	vkFreeCommandBuffers(_device, _commandPool, uint32_t(_commandBuffers.size()), _commandBuffers.data());
	_commandBuffers.clear();

	vkDestroyRenderPass(_device, _renderPass, nullptr);
	for (auto& v : _framebuffers)
	{
		vkDestroyFramebuffer(_device, v, nullptr);
	}
	_framebuffers.clear();

	vkFreeMemory(_device, _depthBufferMemory, nullptr);
	vkDestroyImage(_device, _depthBuffer, nullptr);
	vkDestroyImageView(_device, _depthBufferView, nullptr);

	for (auto& v : _swapchainImageViews)
	{
		vkDestroyImageView(_device, v, nullptr);
	}
	_swapchainImages.clear();
	vkDestroySwapchainKHR(_device, _swapchain, nullptr);

	for (auto& v : _fences)
	{
		vkDestroyFence(_device, v, nullptr);
	}
	_fences.clear();
	vkDestroySemaphore(_device, _presentCompletedSemaphore, nullptr);
	vkDestroySemaphore(_device, _renderCompletedSemaphore, nullptr);

	vkDestroyCommandPool(_device, _commandPool, nullptr);

	vkDestroySurfaceKHR(_instance, _surface, nullptr);
	vkDestroyDevice(_device, nullptr);
#ifdef _DEBUG
	DisableDebugReport();
#endif
	vkDestroyInstance(_instance, nullptr);
}


// Debug report 表示用関数
static VkBool32 VKAPI_CALL DebugReportCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objactTypes,
	uint64_t object,
	size_t	location,
	int32_t messageCode,
	const char* pLayerPrefix,
	const char* pMessage,
	void* pUserData)
{
	std::stringstream ss;
	ss << "[" << pLayerPrefix << "] "  << pMessage << std::endl;

	OutputDebugStringA(ss.str().c_str());

	return VK_FALSE;
}

void AppBase::EnableDebugReport()
{
	// 関数ポインタの取得
	_createDebugReportCallback 
		= reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT"));
	_debugReportMessage 
		= reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(_instance, "vkDebugReportMessageEXT"));
	_destroyDebugReportCallback
		= reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT"));

	VkDebugReportFlagsEXT flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

	VkDebugReportCallbackCreateInfoEXT ci{};
	ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	ci.flags = flags;
	ci.pfnCallback = &DebugReportCallback;
	_createDebugReportCallback(_instance, &ci, nullptr, &_debugReportCallback);
}

void AppBase::DisableDebugReport()
{
	if (_destroyDebugReportCallback)
	{
		_destroyDebugReportCallback(_instance, _debugReportCallback, nullptr);
	}
}