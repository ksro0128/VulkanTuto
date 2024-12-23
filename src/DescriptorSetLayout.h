#pragma once

#include "Common.h"
#include "VulkanContext.h"

class DescriptorSetLayout {
public:
	static std::unique_ptr<DescriptorSetLayout> createDescriptorSetLayout();
	static std::unique_ptr<DescriptorSetLayout> createGammaDescriptorSetLayout();
	~DescriptorSetLayout() {}
	void cleanup();

	VkDescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; }

private:
	VkDescriptorSetLayout descriptorSetLayout;

	void initDescriptorSetLayout();
	void initGammaDescriptorSetLayout();
};
