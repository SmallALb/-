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

从上往下看   querySwapChainSupportInfo()：    这个函数主要是构建一个我门创建的结构体  SwapChainSupportInfo

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

vkGetPhysicalDeviceSurfaceCapabilitiesKHR ;

创建surface的容量信息，

vkGetPhysicalDeviceSurfaceFormatsKHR；

创建格式信息组，并且后面传到  SwapChainSupportInfo  中的 VksurfaceformatKHR 数组中；

vkGetPhysicalDeviceSurfacePresentModesKHR；

创建显示模式，和上面的同理；

最后将他传回去给 swapchain的构造函数中去;


再来看看三个选择：

```cpp
VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupportInfo.mFormats);

VkPresentModeKHR presentMode = chooseSurfacePresentMode(swapChainSupportInfo.mPresentModes);

VkExtent2D extent = chooseExtent(swapChainSupportInfo.mCapabilities);
```

对构建得到的swapChaininfo中的信息中进行挑选；这里不细讲；


然后下面就是创建真正的 交换链信息 VkSwapchainCreateInfoKHR

```cpp

VkSwapchainCreateInfoKHR createInfo = {};
createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
createInfo.surface = mSurface->getSurface();
createInfo.minImageCount = mImageCount;
createInfo.imageFormat = surfaceFormat.format;
createInfo.imageColorSpace = surfaceFormat.colorSpace;
createInfo.imageExtent = extent;

createInfo.imageArrayLayers = 1; 
createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
```

可以看出其实传入的都是些和图片相关的信息格式；


下面再来看看根据队列族来设定的信息

```cpp
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
```

当队列只有一个时，关闭图片资源共享，无需传入队列的信息；

当队列有两个时，开启共享，得到指定的输出队列位置，绑定下信息。


最后再将显示模式，容量的转换信息，剪裁信息传入：

```cpp
createInfo.preTransform = swapChainSupportInfo.mCapabilities.currentTransform;

createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

createInfo.presentMode = presentMode;

createInfo.clipped = VK_TRUE;
```

最后绑定 交换链的信息：

```cpp
vkCreateSwapchainKHR(mDevice->getDevice(), &createInfo, nullptr, &mSwapChain)
```

绑定格式信息和扩展信息,以及绑定交换链的图片信息：

```cpp
mSwapChainFormat = surfaceFormat.format;
mSwapChainExtent = extent;

vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, nullptr);
mSwapChainImages.resize(mImageCount);

vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, mSwapChainImages.data());

mSwapChainImageViews.resize(mImageCount);
for (int i = 0; i < mImageCount; ++i) {
			mSwapChainImageViews[i] = createImageView(mSwapChainImages[i], mSwapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}

```

后面还对交换链的每一个图片信息创建 view 信息属性,，来看看view的属性实现：

```cpp
VkImageView SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;

	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView{ VK_NULL_HANDLE };
	if (vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("Error: failed to create image view in swapchain");
	}

	return imageView;
}
```

Vkimageview 其实就是 给图片传入一个怎么看的信息；

最后的最后来看看整体的代码：

```cpp
#include "swapChain.h"

namespace FF::Wrapper {
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

	SwapChain::~SwapChain() {
		for (auto& imageView : mSwapChainImageViews) {
			vkDestroyImageView(mDevice->getDevice(), imageView, nullptr);
		}

		if (mSwapChain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(mDevice->getDevice(), mSwapChain, nullptr);
		}
		mDevice.reset();
		mSurface.reset();
		mDevice.reset();
	}
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
	VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
			return { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR SwapChain::chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresenstModes) {
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& availablePresenstMode : availablePresenstModes) {
			if (availablePresenstMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresenstMode;
			}
			else if (availablePresenstMode == VK_PRESENT_MODE_FIFO_KHR){
				return bestMode;
			}
		}
	
	}

	VkExtent2D SwapChain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
	
		int width = 0, height = 0;
		glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::max(capabilities.minImageExtent.width, 
			std::min(capabilities.maxImageExtent.width, actualExtent.width));

		actualExtent.height = std::max(capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, actualExtent.height));
	}

	VkImageView SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
	{
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;

		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView{ VK_NULL_HANDLE };
		if (vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create image view in swapchain");
		}

		return imageView;
	}


}

```
