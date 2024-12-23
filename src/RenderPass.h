#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "VulkanUtil.h"

class RenderPass {
public:
	static std::unique_ptr<RenderPass> createRenderPass(VkFormat swapChainImageFormat);
	
	~RenderPass() {}
	void cleanup();

	VkRenderPass getRenderPass() { return renderPass; }

private:
	VkRenderPass renderPass;

	void initRenderPass(VkFormat swapChainImageFormat);
};
