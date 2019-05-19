#include "AppBase.h"

// Create�Ȃǂ̌��ʂ��󂯔���������Ȃ�
void AppBase::CheckResult(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		DebugBreak();
	}
}


// �R���X�g���N�^
AppBase::AppBase() : _presentMode(VK_PRESENT_MODE_FIFO_KHR), _imageIndex(0)
{
}



// �A�v���P�[�V�����̏������������Ȃ�
void AppBase::Initialize(GLFWwindow* window, const char* appName)
{
	// �C���X�^���X�̐���
	InitializeInstance(appName);

	// �����f�o�C�X�̎擾
	GetPhysicalDevice();

	// �O���t�B�b�N�X�p�̃L���[�t�@�~���[�C���f�b�N�X�̎擾
	_graphicsQueueFamilyIndex = SearchGraphicsQueueFamilyIndex();


}


// Instance�𐶐�����
void AppBase::InitializeInstance(const char* appName)
{
	// ApplicationInfo�̏�����
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = appName;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = appName;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	// Instance extension�̎擾
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

	// Instance Create Info�̏�����
	VkInstanceCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ci.pNext = nullptr;
	ci.flags = 0;
	ci.pApplicationInfo = &appInfo;

#ifdef _DEBUG
	// Debug���͉��L��meta layer��L���ɂ���
	const char* layers[] = { "VK_LAYER_LUNARG_standard_validation" };
	ci.enabledLayerCount = 1;
	ci.ppEnabledLayerNames = layers;
#endif

	ci.enabledExtensionCount = uint32_t(extensions.size());
	ci.ppEnabledExtensionNames = extensions.data();

	// Instance�̐���
	auto result = vkCreateInstance(&ci, nullptr, &_instance);
	CheckResult(result);
}


// �����f�o�C�X���擾����
void AppBase::GetPhysicalDevice()
{
	// �ڑ�����Ă��镨���f�o�C�X��񋓂���
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(_instance, &count, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices(count);
	vkEnumeratePhysicalDevices(_instance, &count, physicalDevices.data());

	// �ŏ��̃f�o�C�X�̃������v���p�e�B���擾����
	_physicalDevice = physicalDevices[0];
	vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_physicalDeviceMemoryProperties);
}


// �O���t�B�b�N�X�p�̃L���[�t�@�~���[�C���f�b�N�X���擾����
uint32_t AppBase::SearchGraphicsQueueFamilyIndex()
{
	// �����f�o�C�X�̃L���[�t�@�~���[�v���p�e�B���擾����
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &count, nullptr);
	std::vector<VkQueueFamilyProperties> props(count);
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &count, props.data());

	// �擾�����L���[�t�@�~���[�̂����A�O���t�B�b�N�X�p�̃L���[�t�@�~���[�̃C���f�b�N�X���擾����
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


// �_���f�o�C�X���쐬����
void AppBase::CreateDevice()
{
	const float defaultQueuePriority(1.0f);
	VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.queueFamilyIndex = _graphicsQueueFamilyIndex;
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.pQueuePriorities = &defaultQueuePriority;

	// device extension�̎擾
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

	// Device Create Info �̏�����
	VkDeviceCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	ci.pQueueCreateInfos = &deviceQueueCreateInfo;
	ci.queueCreateInfoCount = 1;
	ci.ppEnabledExtensionNames = extensions.data();
	ci.enabledExtensionCount = uint32_t(extensions.size());



	// �f�o�C�X�̐���
	auto result = vkCreateDevice(_physicalDevice, &ci, nullptr, &_device);
	CheckResult(result);

	// �f�o�C�X�L���[�̎擾
	vkGetDeviceQueue(_device, _graphicsQueueFamilyIndex, 0, &_deviceQueue);
}


// �R�}���h�v�[���̍쐬
void AppBase::CreateCommandPool()
{
	VkCommandPoolCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	ci.queueFamilyIndex = _graphicsQueueFamilyIndex;
	ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	auto result = vkCreateCommandPool(_device, &ci, nullptr, &_commandPool);
	CheckResult(result);
}