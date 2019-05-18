#include "AppBase.h"

// Createなどの結果を受け判定をおこなう
void AppBase::CheckResult(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		DebugBreak();
	}
}


// Instanceを生成する
void AppBase::InitializeInstance(const char* appName)
{
	// ApplicationInfoの初期化
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = appName;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = appName;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	// extensionの取得
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

	// CreateInfoの初期化
	VkInstanceCreateInfo ci;
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
	uint32_t count = 0;

	vkEnumeratePhysicalDevices(_instance, &count, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices(count);
	vkEnumeratePhysicalDevices(_instance, &count, physicalDevices.data());

	_physicalDevice = physicalDevices[0];
	vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_physicalDeviceMemoryProperties);
}