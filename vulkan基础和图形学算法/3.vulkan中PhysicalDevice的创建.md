# PhysicalDevice的创建


先来看看device类的创建

```cpp
namespace FF::Wrapper {
	class Device {
	public:
		using Ptr = std::shared_ptr<Device>;

		static Ptr create(Instance::Ptr instance) { return std::make_shared<Device>(instance); }

		Device(Instance::Ptr instance);

		~Device();

		void pickPhysicalDevice();                                  //得到显卡

		int rateDevice(VkPhysicalDevice device);                    //计算显卡的优先级别

		bool isDeviceSuitable(VkPhysicalDevice device);             //判断显卡的某些特定内容有没有开启
	private:
		VkPhysicalDevice mPhysicalDevice{VK_NULL_HANDLE};
		Instance::Ptr mInstance{ nullptr };
	};

}
```

和instance一样有一个智能指针；

他还要传入一个 Instance 的指针来将两者进行绑定操作；


首先看看如何获取显卡；和加入扩展 组件 一个道理 ；

定义一个计数器；获取有哪些显卡数量；并且定义 一个 vector< VkPhysicalDevice > 存放搜索到的显卡

```cpp
uint32_t deviceCount = 0;
vkEnumeratePhysicalDevices(mInstance->getInstance(), &deviceCount, nullptr);
if (deviceCount == 0) {
	throw std::runtime_error("Eror:failed to enumratePhysicalDevice");
}
std::vector<VkPhysicalDevice> devices(deviceCount);
vkEnumeratePhysicalDevices(mInstance->getInstance(), &deviceCount, devices.data());

```

vkEnumeratePhysicalDevices 传入的参数主要为 要绑定的instance的地址，记录显卡的数量的计数器地址，vector的地址；

接着就是使用一个multimap来存放显卡，这个map会根据显卡的分数来进行排序，高的会优先排在后面。

先来看看显卡的分数如何得到：

```cpp
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
```

主要传入一张显卡；然后调用显卡的参数来判断分数；

比如 VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU 判断显卡是不是独立显卡；

deviceProp.limits.maxImageDimension2D  显卡的最大采样纹理数；

deviceFeatures.geometryShader  支不支持几何着色器， 不支持直接剔除分数；

对计算完成的显卡加上；以键值对的方式推进map中：

```cpp
std::multimap<int, VkPhysicalDevice> candidates;
for (const auto& device : devices) {
	int score = rateDevice(device);
	candidates.insert(std::make_pair(score, device));
}
```

最后就是判断显卡符不符合要求：

```cpp
bool Device::isDeviceSuitable(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProp;
	vkGetPhysicalDeviceProperties(device, &deviceProp);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	return deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
}
```

和分数计算大差不差；要根据情况来制定；

最后再绑定要启动的显卡：

```cpp
if (candidates.rbegin()->first > 0 && isDeviceSuitable(candidates.rbegin()->second)) {
	mPhysicalDevice = candidates.rbegin()->second;
}

if (mPhysicalDevice == VK_NULL_HANDLE) {
	throw::std::runtime_error("Error:failed to physical device");
}
```

最后的最后来看看完整的cpp源码：

```cpp
#include "device.h"

namespace FF::Wrapper {
	Device::Device(Instance::Ptr instance) {
		mInstance = instance;
		pickPhysicalDevice();
	}

	Device::~Device(){
		mInstance.reset();
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


}
```
