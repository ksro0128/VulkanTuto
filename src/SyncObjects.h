#pragma once

#include "Common.h"
#include "VulkanContext.h"

class SyncObjects {
public:
	static std::unique_ptr<SyncObjects> createSyncObjects();
	~SyncObjects() {}
	void cleanup();

	std::vector<VkSemaphore>& getImageAvailableSemaphores() { return imageAvailableSemaphores; }
	std::vector<VkSemaphore>& getRenderFinishedSemaphores() { return renderFinishedSemaphores; }
	std::vector<VkFence>& getInFlightFences() { return inFlightFences; }

private:
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	void initSyncObjects();
};
