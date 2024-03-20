# VulKan的交换链

先来看看交换链类的创建：

```cpp
#pragma once

#include "../base.h"
#include "device.h"
#include "windows.h"
#include "windowSurface.h"

namespace FF::Wrapper {

	struct SwapChainSupportInfo {
		VkSurfaceCapabilitiesKHR mCapabilities;
		std::vector<VkSurfaceFormatKHR> mFormats;
		std::vector<VkPresentModeKHR> mPresentModes;
	};

	class SwapChain {
	public:
		using Ptr = std::shared_ptr<SwapChain>;

		static Ptr create(const Device::Ptr& device, const Window::Ptr& window, const WindowSurface::Ptr& surface) {
			return std::make_shared<SwapChain>(device, window, surface);
		}

		SwapChain(const Device::Ptr &device, const Window::Ptr &window, const WindowSurface::Ptr &surface);

		~SwapChain();

		SwapChainSupportInfo querySwapChainSupportInfo();

		VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

		VkPresentModeKHR chooseSurfacePresentMode(const std::vector<VkPresentModeKHR> &availablePresenstModes);

		VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	private:
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels=0);

	private:
		VkSwapchainKHR mSwapChain{VK_NULL_HANDLE};

		VkFormat mSwapChainFormat;
		VkExtent2D mSwapChainExtent;
		uint32_t mImageCount{ 0 };

		std::vector<VkImage> mSwapChainImages{};
		std::vector<VkImageView> mSwapChainImageViews{};

		Device::Ptr mDevice{nullptr};
		Window::Ptr mWindow{nullptr};
		WindowSurface::Ptr mSurface{nullptr};
	};

}


```

这次涉及的变量东西特别多，因此首先看看定义的对象里面哪些对象，一个一个解释清楚：

```cpp
VkSwapchainKHR mSwapChain{VK_NULL_HANDLE};
VkFormat mSwapChainFormat;
VkExtent2D mSwapChainExtent;
uint32_t mImageCount{ 0 };
std::vector<VkImage> mSwapChainImages{};
std::vector<VkImageView> mSwapChainImageViews{};

```

VkSwapchainKHR：这个就是在vulkan里面的交换链；

VkFormat：表示图像或缓冲区数据；

VkExtent2D：表示二维图像或区域的尺寸；

std::vector< VkImage >：图像对象的数据结构 在这里使用了这个对象的数组；

std::vector< VkImageView >：描述了如何访问 VkImage 中的数据 在这里使用了这个对象的数组；

每一个对象涉及到的东西都及其的多，之间的关联又及其的密集；但还是一步一步的看。

先从 构造来看：

```cpp
SwapChain::SwapChain(const Device::Ptr& device, const Window::Ptr& window, const WindowSurface::Ptr& surface) {
	mDevice = device;
	mWindow = window;
	mSurface = surface;

	auto swapChainSupportInfo = querySwapChainSupportInfo();

	VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupportInfo.mFormats);

	VkPresentModeKHR presentMode = chooseSurfacePresentMode(swapChainSupportInfo.mPresentModes);

	VkExtent2D extent = chooseExtent(swapChainSupportInfo.mCapabilities);

	mImageCount = swapChainSupportInfo.mCapabilities.minImageCount + 1;

	if (swapChainSupportInfo.mCapabilities.maxImageCount > 0 && mImageCount > swapChainSupportInfo.mCapabilities.maxImageCount) {
		mImageCount = swapChainSupportInfo.mCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = mSurface->getSurface();
	createInfo.minImageCount = mImageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;

	createInfo.imageArrayLayers = 1; 
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


	std::vector<uint32_t> queueFamiles = { mDevice->getGraphicQueueFamily().value(), mDevice->getPresentQueueFamily().value()};
	if (mDevice->getGraphicQueueFamily() == mDevice->getPresentQueueFamily()) {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamiles.size());
		createInfo.pQueueFamilyIndices = queueFamiles.data();
	}

	createInfo.preTransform = swapChainSupportInfo.mCapabilities.currentTransform;

	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;

	createInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(mDevice->getDevice(), &createInfo, nullptr, &mSwapChain) != VK_SUCCESS) {
		throw std::runtime_error("Error: failed to create swapChain");
	}

	mSwapChainFormat = surfaceFormat.format;
	mSwapChainExtent = extent;

	vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, nullptr);
	mSwapChainImages.resize(mImageCount);

	vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, mSwapChainImages.data());

	mSwapChainImageViews.resize(mImageCount);
	for (int i = 0; i < mImageCount; ++i) {
		mSwapChainImageViews[i] = createImageView(mSwapChainImages[i], mSwapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}
```

从上往下看   querySwapChainSupportInfo()：

```cpp
SwapChainSupportInfo SwapChain::querySwapChainSupportInfo() {
		SwapChainSupportInfo info;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &info.mCapabilities);

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &formatCount, nullptr);

		if (formatCount != 0) {
			info.mFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &formatCount, info.mFormats.data());

		}

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			info.mPresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, info.mPresentModes.data());

		}

		return info;
	}
```


vkGetPhysicalDeviceSurfaceCapabilitiesKHR  ( VkPhysicalDevice,   VkSurfaceKHR,    VkSurfaceCapabilitiesKHR\*   );

创建surface的容量信息
