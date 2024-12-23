#include "Texture.h"

std::shared_ptr<Texture> Texture::createTexture(std::string path) {
    std::shared_ptr<Texture> texture = std::shared_ptr<Texture>(new Texture());
    texture->initTexture(path);
    return texture;
}


void Texture::cleanup() {
    auto& context = VulkanContext::getContext();
    auto device = context.getDevice();

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);
    m_imageBuffer->cleanup();
}


void Texture::initTexture(std::string path) {
    loadTexture(path);
}


void Texture::loadTexture(std::string path) {
    m_imageBuffer = ImageBuffer::createImageBuffer(path);
    mipLevels = m_imageBuffer->getMipLevels();
    createTextureImageView();
    createTextureSampler();
}


// 텍스처 이미지 뷰 생성
void Texture::createTextureImageView() {
    // SRGB 총 4바이트 포맷으로 된 이미지 뷰 생성
    textureImageView = VulkanUtil::createImageView(m_imageBuffer->getImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_imageBuffer->getMipLevels());
}


// 텍스처를 위한 샘플러 생성 함수
void Texture::createTextureSampler() {
    auto& context = VulkanContext::getContext();
    auto device = context.getDevice();
    auto physicalDevice = context.getPhysicalDevice();

    // GPU의 속성 정보를 가져오는 함수
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    // 샘플러 생성시 필요한 구조체 
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;							// 확대시 필터링 적용 설정 (현재 선형 보간 필터링 적용)
    samplerInfo.minFilter = VK_FILTER_LINEAR;							// 축소시 필터링 적용 설정 (현재 선형 보간 필터링 적용)
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;			// 텍스처 좌표계의 U축(너비)에서 범위를 벗어난 경우 래핑 모드 설정 (현재 반복 설정) 
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;			// 텍스처 좌표계의 V축(높이)에서 범위를 벗어난 경우 래핑 모드 설정 (현재 반복 설정) 
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;			// 텍스처 좌표계의 W축(깊이)에서 범위를 벗어난 경우 래핑 모드 설정 (현재 반복 설정) 
    samplerInfo.anisotropyEnable = VK_TRUE;								// 이방성 필터링 적용 여부 설정 (경사진 곳이나 먼 곳의 샘플을 늘려 좀 더 정확한 값을 가져오는 방법)
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;	// GPU가 지원하는 최대의 이방성 필터링 제한 설정
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;			// 래핑 모드가 VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER 일때 텍스처 외부 색 지정
    samplerInfo.unnormalizedCoordinates = VK_FALSE;						// VK_TRUE로 설정시 텍스처 좌표가 0 ~ 1의 정규화된 좌표가 아닌 실제 텍셀의 좌표 범위로 바뀜
    samplerInfo.compareEnable = VK_FALSE;								// 비교 연산 사용할지 결정 (보통 쉐도우 맵같은 경우 깊이 비교 샘플링에서 사용됨)
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;						// 비교 연산에 사용할 연산 지정
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;				// mipmap 사용시 mipmap 간 보간 방식 결정 (현재 선형 보간)
    samplerInfo.minLod = 0.0f;											// 최소 level을 0으로 설정 (가장 높은 해상도의 mipmap 을 사용가능하게 허용)
    samplerInfo.maxLod = static_cast<float>(m_imageBuffer->getMipLevels());					// 최대 level을 mipLevel로 설정 (VK_LOD_CLAMP_NONE 설정시 제한 해제)
    samplerInfo.mipLodBias = 0.0f;										// Mipmap 레벨 오프셋(Bias)을 설정 
                                                                        // Mipmap을 일부러 더 높은(더 큰) 레벨로 사용하거나 낮은(더 작은) 레벨로 사용하고 싶을 때 사용.

    // 샘플러 생성
    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}
