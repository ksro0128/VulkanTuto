#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "Pipeline.h"
#include "CommandBuffers.h"
#include "FrameBuffers.h"
#include "SyncObjects.h"
#include "ShaderResourceManager.h"

class Renderer {
public:
	static std::unique_ptr<Renderer> createRenderer(GLFWwindow* window);
	~Renderer() {}
	void cleanup();

	void loadScene(Scene* scene);
	void drawFrame(Scene* scene);
	void recreateSwapChain();

	VkDevice getDevice() { return device; }

private:
	Renderer() {}

	GLFWwindow* window;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkDevice device;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	std::unique_ptr<SwapChain> m_swapChain;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	std::unique_ptr<FrameBuffers> m_swapChainFrameBuffers;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::unique_ptr<RenderPass> m_renderPass;
	VkRenderPass renderPass;
	std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
	VkDescriptorSetLayout descriptorSetLayout;
	std::unique_ptr<Pipeline> m_pipeline;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	std::vector< std::shared_ptr<UniformBuffer> > m_uniformBuffers;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
	std::unique_ptr<ShaderResourceManager> m_shaderResourceManager;
	std::unique_ptr<CommandBuffers> m_commandBuffers;
	std::vector<VkCommandBuffer> commandBuffers;
	std::unique_ptr<SyncObjects> m_syncObjects;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	uint32_t currentFrame = 0;


	void init(GLFWwindow* window);
	void recordCommandBuffer(Scene* scene, VkCommandBuffer commandBuffer, uint32_t imageIndex);
};
