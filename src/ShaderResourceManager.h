#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "Buffer.h"
#include "Scene.h"

class ShaderResourceManager {
public:
	static std::unique_ptr<ShaderResourceManager> createShaderResourceManager(Scene* scene, VkDescriptorSetLayout descriptorSetLayout);
	~ShaderResourceManager() {}
	void cleanup();

	std::vector<std::shared_ptr<UniformBuffer>>& getUniformBuffers() { return m_uniformBuffers; }
	std::vector<VkDescriptorSet>& getDescriptorSets() { return descriptorSets; }

private:
	std::vector<std::shared_ptr<UniformBuffer>> m_uniformBuffers;
	std::vector<VkDescriptorSet> descriptorSets;

	void initShaderResourceManager(Scene* scene, VkDescriptorSetLayout descriptorSetLayout);
	void createUniformBuffers(Scene* scene);
	void createDescriptorSets(Scene* scene, VkDescriptorSetLayout descriptorSetLayout);
};
