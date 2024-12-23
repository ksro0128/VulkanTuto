#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "VulkanUtil.h"

class SwapChain {
public:
	static std::unique_ptr<SwapChain> createSwapChain(GLFWwindow* window);
    ~SwapChain() {}
	void cleanup();

	void recreateSwapChain();

	VkSwapchainKHR getSwapChain() { return swapChain; }
	std::vector<VkImage>& getSwapChainImages() { return swapChainImages; }
	VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
	VkExtent2D getSwapChainExtent() { return swapChainExtent; }
	std::vector<VkImageView>& getSwapChainImageViews() { return swapChainImageViews; }

private:
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;

	GLFWwindow* window;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkSurfaceKHR surface;

	void initSwapChain(GLFWwindow* window);
	void createSwapChain();
	void createImageViews();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};
