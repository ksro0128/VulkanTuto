#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "VulkanUtil.h"

class Pipeline {
public:
	static std::unique_ptr<Pipeline> createPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	~Pipeline() {}
	void cleanup();

	VkPipeline getPipeline() { return pipeline; }
	VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

private:
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	void initPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	VkShaderModule createShaderModule(const std::vector<char>& code);
};
