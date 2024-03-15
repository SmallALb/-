# instance的创建和各类细节的处理

## instance创建

首先要创建一个Instance类方便对接 application

创建的代码如下：

```cpp
namespace FF::Wrapper {
	class Instance {
	public:
		using Ptr = std::shared_ptr<Instance>;



		static Ptr create(bool enableValidationLayer) {
	
			return std::make_shared<Instance>(enableValidationLayer);
		}

		Instance(bool enableValidationLayer);

		~Instance();

		void printAvailableExtensions();

		std::vector<const char*> getRequiredExtensions();

		bool checkValidationLayerSupport();

		void setupDebugger();
		 
	private:
		VkInstance mInstance;
		bool mEnableValidationLayer{ false };
		VkDebugUtilsMessengerEXT mDebugger;
	};

}
```

这里使用了智能指针来创建这个对象，让其自动进行释放防止内除溢出；

定义一个static类 create 返回这个Instance类的智能指针类型；

这里私有成员变量  VkInstance 就是要创建的 Instance ；对于该instance 我们要传入许多的数据；

首先是 VkApplicationInfo 对vulkan这个应用的各类设置：

```cpp
VkApplicationInfo appInfo = {};
appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;                        //首先要告诉他是什么类型
appInfo.pApplicationName = "vulkanLession";                                //设定应用的名字
appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);                     //设定应用版本
appInfo.pEngineName = "NO ENGINE";                                         //设定Engine的名字
appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);                          //设定engine的版本
appInfo.apiVersion = VK_API_VERSION_1_0;                                   //设定api的版本
```

然后就是  VkInstanceCreateInfo  对instance的建立信息进行设置；

对于 VkInstanceCreateInfo 有如下要创建：

* 类型
* 关于这个instance应用的指针
* 添加相关的扩展（扩展的数量，添加扩展名字的数据）
* 可选组件Layer（这里先使用Debug组件）

首先是最基础的类型  和 instance应用指针的创建

```cpp
VkInstanceCreateInfo instCreateInfo = {};
instCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
instCreateInfo.pApplicationInfo = &appInfo;                                //要得到VkApplicationInfo的应用信息
```

再接着就是扩展的创建；

在这里， 我们需要一个字符串vector来让createinfo接收；

这里定义了   getRequiredExtensions  函数来进行构建：

```cpp
std::vector<const char*> Instance::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	return extensions;
}
```

注：这里的glfwGetRequiredInstanceExtensions( )是为了拿到和glfw有关联的组件名称；

函数返回一个    vector<const char*>   glfwGetRequiredInstanceExtensions( );里面要传入一个 uint32_t 类型的地址，目的是为了 得到要添加扩展的数量；

然后定义一个 const char**  接收 glfwGetRequiredInstanceExtensions 返回的二维数组（其实就是字符串数组）；再使用 vector创建的(arr.begin(), arr.end())来把数据传入 我们创建的vector<const char*>；

这里多传入了一个 VK_EXT_DEBUG_UTILS_EXTENSION_NAME；添加了一个debug扩展；为后面启动debug做准备；

然后就能靠自定义的 glfwGetRequiredInstanceExtensions  函数来创建数据了：

```cpp
auto extensions = getRequiredExtensions();
instCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
instCreateInfo.ppEnabledExtensionNames = extensions.data();
```

enabledExtensionCount   传入扩展的数量 （要转化为uint32_t）

ppEnabledExtensionNames 传入数据  （使用了vector 的data()    能够返回一个数组的首地址）

然后就是 layer 的开启了，这里首先定义一个 二维数组 存储 layer 的 字符信息：

```cpp
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};
```

为了确定layer是否开启，在instance中添加了一个bool变量；

bool mEnableValidationLayer{ false }；初始他为false；然后在application类中初始化的时候 create(   ) 传入一个bool值；手动的来决定 开启/关闭 Layer

```cpp
bool mEnableValidationLayer{ false };
```

这里还需要定义一个checkValidationLayerSupport函数来判断  validationLayers 中里面有什么功能：

```cpp
bool Instance::checkValidationLayerSupport() {
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const auto& layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProp : availableLayers) {
			if (std::strcmp(layerName, layerProp.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}
```

最后再填上这段抛出代码，防止没加载：

```cpp
if (mEnableValidationLayer && !checkValidationLayerSupport()) {
	throw std::runtime_error("Error: validation layer is not supported");
}
```

至此，instance 声明完毕；

最后，再将创建的 createinfort 传入 instance中

```cpp
if (vkCreateInstance(&instCreateInfo, nullptr, &mInstance) != VK_SUCCESS) {
	throw std::runtime_error("Error:failed to create instance");
}
```

## 启动DeBug layer

在类里面主要创建了一个 VkDebugUtilsMessengerEXT来接收报错信息：

```cpp
VkDebugUtilsMessengerEXT mDebugger;
```

和创建instance一样，需要自己的createinfo—————— VkDebugUtilsMessengerCreateInfoEXT；在这里会定义一个函数：

```cpp
void Instance::setupDebugger() {
	if (!mEnableValidationLayer) { return; }
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	createInfo.messageType = 
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	createInfo.pfnUserCallback = debugCallBack;
	createInfo.pUserData = nullptr;

	if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugger) != VK_SUCCESS) {
		throw std::runtime_error("Error:failed to create debugger");
	}
}
```

messageSeverity主要就是开启各种校验层，

messageType主要就是告诉返回的类型值

pfnUserCallback是设置一个回调函数，接收一个函数的指针

pUserData主要就是接收用户给到的数据，在debug中这里先为空；

然后将debug数据传给instance；

再构建下回调函数；和辅助的函数：

```cpp
	//validation layer 回调函数
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pMessageData,
		void* pUserData) {
		std::cout << "ValidationLayer: " << pMessageData->pMessage << std::endl;

		return VK_FALSE;
	}

	//辅助函数
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* debugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,         "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, debugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
```

最后再给instanceCreateinfo传入layer的信息：

```cpp
if (mEnableValidationLayer) {
	instCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	instCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		instCreateInfo.enabledLayerCount = 0;
}
```

最后的最后来看下完整的代码：

```cpp
#include "instance.h"

namespace FF::Wrapper {
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	//validation layer 回调函数
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pMessageData,
		void* pUserData) {
		std::cout << "ValidationLayer: " << pMessageData->pMessage << std::endl;

		return VK_FALSE;
	}

	//辅助函数
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* debugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, debugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT  debugMessenger,
		const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr) {
			return func(instance, debugMessenger, pAllocator);
		}
	}


	Instance::Instance(bool enableValidationLayer) {
		mEnableValidationLayer = enableValidationLayer;

		if (mEnableValidationLayer && !checkValidationLayerSupport()) {
			throw std::runtime_error("Error: validation layer is not supported");
		}

		printAvailableExtensions();

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "vulkanLession";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "NO ENGINE";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo instCreateInfo = {};
		instCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instCreateInfo.pApplicationInfo = &appInfo;

		//扩展相关
		auto extensions = getRequiredExtensions();
		instCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instCreateInfo.ppEnabledExtensionNames = extensions.data();

		//layer相关
		if (mEnableValidationLayer) {
			instCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			instCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			instCreateInfo.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&instCreateInfo, nullptr, &mInstance) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create instance");
		}

		setupDebugger();
	}

	Instance::~Instance() {
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugger, nullptr);
		vkDestroyInstance(mInstance, nullptr);
	}

	void Instance::printAvailableExtensions() {
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "Available extensions:" << std::endl;

		for (const auto& extension : extensions) {
			std::cout << extension.extensionName << std::endl;
		}
	}

	std::vector<const char*> Instance::getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;

		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return extensions;
	}

	bool Instance::checkValidationLayerSupport() {
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const auto& layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProp : availableLayers) {
				if (std::strcmp(layerName, layerProp.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	void Instance::setupDebugger() {
		if (!mEnableValidationLayer) { return; }

		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		createInfo.messageType = 
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		createInfo.pfnUserCallback = debugCallBack;
		createInfo.pUserData = nullptr;

		if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugger) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create debugger");
		}
	}
}
```

整理完毕。一下就是 instance 和 debug的创建
