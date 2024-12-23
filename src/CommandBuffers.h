#pragma once

#include "Common.h"
#include "VulkanContext.h"

class CommandBuffers {
public:
	static std::unique_ptr<CommandBuffers> createCommandBuffers();
	~CommandBuffers() {}
	void cleanup();

	std::vector<VkCommandBuffer>& getCommandBuffers() { return commandBuffers; }

private:
	std::vector<VkCommandBuffer> commandBuffers;

	void initCommandBuffers();
};
