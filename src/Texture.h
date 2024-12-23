#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "Buffer.h"

class Texture : public Buffer {
public:
	static std::shared_ptr<Texture> createTexture(std::string path);
	~Texture() {}
	void cleanup();

	VkImageView getImageView() { return textureImageView; }
	VkSampler getSampler() { return textureSampler; }
private:
	Texture() {}

	uint32_t mipLevels;
	std::unique_ptr<ImageBuffer> m_imageBuffer;
	VkImageView textureImageView;
    VkSampler textureSampler;

	void initTexture(std::string path);
	void loadTexture(std::string path);
	void createTextureImageView();
	void createTextureSampler();
};
