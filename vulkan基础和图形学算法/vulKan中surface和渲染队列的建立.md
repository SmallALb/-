# Surface和队列族的创立

这里对将窗体包装成了一个windows类：

```cpp
#pragma once

#include "../base.h"

namespace FF::Wrapper {
	class Window {
	public:
		using Ptr = std::shared_ptr<Window>;

		static Ptr create(const int& width, const int& height) { 
			return std::make_shared<Window>(width, height); 
		}

		Window(const int& width, const int& height);

		~Window();

		bool shouldClose();

		void pollEvents();

		[[nodiscard]] auto getWindow() const { return mWindow; }
	private:
		GLFWwindow* mWindow{ nullptr };
		int mWidth{ 0 };
		int mHeight{ 0 };
	};

}
```

```cpp
#include "windowSurface.h"

namespace FF::Wrapper {

	WindowSurface::WindowSurface(Instance::Ptr instance, Window::Ptr window) {
		mInstance = instance;
		if (glfwCreateWindowSurface(instance->getInstance(), window->getWindow(), nullptr, &mSurface) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create surface");
		}
	}

	WindowSurface::~WindowSurface() {
		vkDestroySurfaceKHR(mInstance->getInstance(), mSurface, nullptr);
		mInstance.reset();
	}
}
```

以方便后期的调用和调试；

## 创立Surface

先来看看定义的 Surface 类：

```cpp
#pragma once

#include "../base.h"
#include "instance.h";
#include "windows.h"
namespace FF::Wrapper {
	class WindowSurface {
	public:
		using Ptr = std::shared_ptr<WindowSurface>;

		static Ptr create(Instance::Ptr instance, Window::Ptr window) {
			return std::make_shared<WindowSurface>(instance, window);
		}

		WindowSurface(Instance::Ptr instance, Window::Ptr window);

		~WindowSurface();

		[[nodiscard]] auto getSurface() const { return mSurface; }
	private:
		VkSurfaceKHR mSurface{ VK_NULL_HANDLE };
		Instance::Ptr mInstance{ nullptr }; 
	};



}
```

来看看surface的构造函数：

```cpp
WindowSurface::WindowSurface(Instance::Ptr instance, Window::Ptr window) {
	mInstance = instance;
	if (glfwCreateWindowSurface(instance->getInstance(), window->getWindow(), nullptr, &mSurface) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create surface");
	}
}
```

这里调用了glfwCreateWindowSurface，一个专门为glfw窗体绑定instance和创建surface的函数；传入

一个该程序的  instance  传入一个glfw窗体  传入 VkSurfaceKHR  这个是vulkan的surface类型

这样窗体的surface就创建好了。

## 创建队列

先来看看device类：

```cpp
#pragma once

#include "../base.h"
#include "instance.h"
#include "windowSurface.h"
namespace FF::Wrapper {
	class Device {
	public:
		using Ptr = std::shared_ptr<Device>;

		static Ptr create(Instance::Ptr instance, WindowSurface::Ptr Surface) { return std::make_shared<Device>(instance,Surface); }

		Device(Instance::Ptr instance, WindowSurface::Ptr Surface);

		~Device();

		void pickPhysicalDevice();

		int rateDevice(VkPhysicalDevice device);

		bool isDeviceSuitable(VkPhysicalDevice device);

		void initQueueFamiles(VkPhysicalDevice device);

		void createLogicalDevice();

		bool isQueueFamilyComplete();
	private:
		VkPhysicalDevice mPhysicalDevice{VK_NULL_HANDLE};
		Instance::Ptr mInstance{ nullptr };
		std::optional<uint32_t> mGraphicQueueFamily;
		VkQueue mGraphicQueue{ VK_NULL_HANDLE };
		WindowSurface::Ptr mSurface{nullptr};
		std::optional<uint32_t> mPresentQueueFamily;
		VkQueue mPresentQueue{ VK_NULL_HANDLE };

		VkDevice mDevice{ VK_NULL_HANDLE };
	};

}
```

该类添加了 两个 VkQueue     mGraphicQueue （渲染队列） 和   mPresentQueue（显示队列）

添加了一个Windowssurface对象；

队列序族的编号  用optional来定义，防止出现未知值；

和一个 VkDevice；

同时添加三个函数

```
void initQueueFamiles(VkPhysicalDevice device);          //初始化  队列族
void createLogicalDevice();
bool isQueueFamilyComplete();
```

先来看第一个 初始化  队列族 ：

```cpp
void Device::initQueueFamiles(VkPhysicalDevice device) {
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
			mGraphicQueueFamily = i;
		}
		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface->getSurface(),&presentSupport);

		if (presentSupport) {
		    mPresentQueueFamily = i;
		}
		if (isQueueFamilyComplete()) break;
		++i; 
	}
}
```

老样子，设定计数器，绑定计数器，传入自己的显卡设备；然后定义vector< VkQueueFamilyProperties >数组，把创建的队列族传进去；

先来得到渲染队列的队列族编号；将 VkQueueFamilyProperties 数组中的 队列族给遍历一遍；在数组中的每一个VkQueueFamilyProperties 对象中的队列计数都得大于0   使用 &（位运算的与）  来判断  queueFlag 是不是计算图形的队列；如果是就记录当前的队列族编号；

再来设定个   VkBool32   来判断屏幕渲染队列是否开启；使用vkGetPhysicalDeviceSurfaceSupportKHR() 函数；

传入一个 device 当前遍历的编号  surface  和  要一个  VkBool32 接收他的判断；

再写一个  判断队列编号绑定是否完成的函数：

```cpp
bool Device::isQueueFamilyComplete() {
	return mGraphicQueueFamily.has_value() && mPresentQueueFamily.has_value();
}
```

现在队列准备好了，该将创基队列信息 和 Device进行绑定操作了：

```cpp
void Device::createLogicalDevice() {
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> queueFamilies = {mGraphicQueueFamily.value(), mPresentQueueFamily.value()};
	float queuePriority = 1.0;

	for (uint32_t queueFamily : queueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeratures = {};

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pEnabledFeatures = &deviceFeratures;
	deviceCreateInfo.enabledExtensionCount = 0;

	if (mInstance->getEnableValidationLayer()) {
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t> (validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		deviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice) != VK_SUCCESS) {
		throw std::runtime_error("Error: failed to create logical device");
	}

	vkGetDeviceQueue(mDevice, mGraphicQueueFamily.value(), 0, &mGraphicQueue);
	vkGetDeviceQueue(mDevice, mPresentQueueFamily.value(), 0, &mPresentQueue);
}
```


看起来比较复杂，其实套路早就熟悉；std::vector< VkDeviceQueueCreateInfo > queueCreateInfos; 创建一个专门存放队列信息的数组，用float 存放队列的优先级别，将队列族的编号信息存入一个哈希表中：

```cpp
std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
std::set<uint32_t> queueFamilies = {mGraphicQueueFamily.value(), mPresentQueueFamily.value()};
float queuePriority = 1.0;
```

遍历每一个编号，创建队列信息：

```cpp
for (uint32_t queueFamily : queueFamilies) {
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = queueFamily;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	queueCreateInfos.push_back(queueCreateInfo);
}
```

再将这些队列、逻辑设备绑定到 Device信息中：

```cpp
VkPhysicalDeviceFeatures deviceFeratures = {};

VkDeviceCreateInfo deviceCreateInfo = {};
deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
deviceCreateInfo.pEnabledFeatures = &deviceFeratures;
deviceCreateInfo.enabledExtensionCount = 0;
```

最后将layer层与device绑定， 将device信息与device绑定，为两个队列族填入队列：

```cpp
if (mInstance->getEnableValidationLayer()) {
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t> (validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		deviceCreateInfo.enabledLayerCount = 0;
	}
	if (vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice) != VK_SUCCESS) {
		throw std::runtime_error("Error: failed to create logical device");
	}
	vkGetDeviceQueue(mDevice, mGraphicQueueFamily.value(), 0, &mGraphicQueue);
	vkGetDeviceQueue(mDevice, mPresentQueueFamily.value(), 0, &mPresentQueue);
}
```

最后来看看完整cpp代码：

```cpp
#include "device.h"

namespace FF::Wrapper {
	Device::Device(Instance::Ptr instance, WindowSurface::Ptr Surface) {
		mSurface = Surface;
		mInstance = instance;
		pickPhysicalDevice();
		initQueueFamiles(mPhysicalDevice);
		createLogicalDevice();
	}

	Device::~Device(){
		vkDestroyDevice(mDevice, nullptr);
		mInstance.reset();
		mSurface.reset();
	}

	void Device::pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(mInstance->getInstance(), &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("Eror:failed to enumratePhysicalDevice");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(mInstance->getInstance(), &deviceCount, devices.data());

		std::multimap<int, VkPhysicalDevice> candidates;
		for (const auto& device : devices) {
			int score = rateDevice(device);
			candidates.insert(std::make_pair(score, device));
		}

		if (candidates.rbegin()->first > 0 && isDeviceSuitable(candidates.rbegin()->second)) {
			mPhysicalDevice = candidates.rbegin()->second;
		}

		if (mPhysicalDevice == VK_NULL_HANDLE) {
			throw::std::runtime_error("Error:failed to physical device");
		}
	}

	int Device::rateDevice(VkPhysicalDevice device) {
		int score = 0;

		VkPhysicalDeviceProperties deviceProp;
		vkGetPhysicalDeviceProperties(device, &deviceProp);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		if (deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		score += deviceProp.limits.maxImageDimension2D;

		if (!deviceFeatures.geometryShader) {
			return 0;
		}

		return score;
	}

	bool Device::isDeviceSuitable(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties deviceProp;
		vkGetPhysicalDeviceProperties(device, &deviceProp);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		return deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
	}

	void Device::initQueueFamiles(VkPhysicalDevice device) {
		uint32_t queueFamilyCount = 0;

		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
				mGraphicQueueFamily = i;
			}

			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface->getSurface(), &presentSupport);

			if (presentSupport) {
				mPresentQueueFamily = i;
			}

			if (isQueueFamilyComplete()) break;

			++i; 
		}
	}

	void Device::createLogicalDevice() {
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> queueFamilies = {mGraphicQueueFamily.value(), mPresentQueueFamily.value()};
		float queuePriority = 1.0;

		for (uint32_t queueFamily : queueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}
		//VkDeviceQueueCreateInfo queueCreateInfo = {};
		//queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		//queueCreateInfo.queueFamilyIndex = mGraphicQueueFamily.value();
		//queueCreateInfo.queueCount = 1;


		//queueCreateInfo.pQueuePriorities = &queuePriority;
		VkPhysicalDeviceFeatures deviceFeratures = {};

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pEnabledFeatures = &deviceFeratures;
		deviceCreateInfo.enabledExtensionCount = 0;

		if (mInstance->getEnableValidationLayer()) {
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t> (validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			deviceCreateInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create logical device");
		}

		vkGetDeviceQueue(mDevice, mGraphicQueueFamily.value(), 0, &mGraphicQueue);
		vkGetDeviceQueue(mDevice, mPresentQueueFamily.value(), 0, &mPresentQueue);
	}

	bool Device::isQueueFamilyComplete() {
		return mGraphicQueueFamily.has_value() && mPresentQueueFamily.has_value();
	}


}
```
