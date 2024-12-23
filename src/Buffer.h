#pragma once

#include "Common.h"
#include "VulkanContext.h"
#include "VulkanUtil.h"

class Buffer {
public:
	virtual ~Buffer() {}
	virtual void cleanup() = 0;

protected:
	VkBuffer m_buffer;
	VkDeviceMemory m_bufferMemory;

	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkCommandPool m_commandPool;
	VkQueue m_graphicsQueue;

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};


class VertexBuffer : public Buffer {
public:
	static std::unique_ptr<VertexBuffer> createVertexBuffer(std::vector<Vertex>& vertices);
	~VertexBuffer() {}
	void cleanup();

	void bind(VkCommandBuffer commandBuffer);

private:
	void initVertexBuffer(std::vector<Vertex>& vertices);
};


class IndexBuffer : public Buffer {
public:
	static std::unique_ptr<IndexBuffer> createIndexBuffer(std::vector<uint32_t>& indices);
	~IndexBuffer() {}
	void cleanup();

	void bind(VkCommandBuffer commandBuffer);

	uint32_t getIndexCount() { return m_indexCount; }

private:
	uint32_t m_indexCount;

	void initIndexBuffer(std::vector<uint32_t>& indices);
};


class ImageBuffer : public Buffer {
public:
	static std::unique_ptr<ImageBuffer> createImageBuffer(std::string path);
	~ImageBuffer() {}
	void cleanup();

	uint32_t getMipLevels() { return mipLevels; }
	VkImage getImage() { return textureImage; }
	VkDeviceMemory getImageMemory() { return textureImageMemory; }

private:
	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;

	void initImageBuffer(std::string path);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
};

class UniformBuffer : public Buffer {
public:
	static std::shared_ptr<UniformBuffer> createUniformBuffer(VkDeviceSize buffersize);
	~UniformBuffer() {}
	void cleanup();

	void updateUniformBuffer(void* data, VkDeviceSize size);

	VkBuffer getBuffer() { return m_buffer; }
	VkDeviceMemory getBufferMemory() { return m_bufferMemory; }
private:
	void* m_mappedMemory = nullptr;

	void initUniformBuffer(VkDeviceSize buffersize);
};
