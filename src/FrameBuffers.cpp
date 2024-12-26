#include "FrameBuffers.h"

std::unique_ptr<FrameBuffers> FrameBuffers::createSwapChainFrameBuffers(SwapChain* swapChain, VkRenderPass renderPass) {
    std::unique_ptr<FrameBuffers> frameBuffers = std::unique_ptr<FrameBuffers>(new FrameBuffers());
    frameBuffers->initSwapChainFrameBuffers(swapChain, renderPass);
    return frameBuffers;
}


void FrameBuffers::cleanup() {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    vkDestroyImageView(device, colorImageView, nullptr);
    vkDestroyImage(device, colorImage, nullptr);
    vkFreeMemory(device, colorImageMemory, nullptr);

    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);

    vkDestroyImageView(device, resolveImageView, nullptr);
    vkDestroyImage(device, resolveImage, nullptr);
    vkFreeMemory(device, resolveImageMemory, nullptr);

    for (auto framebuffer : framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
}


void FrameBuffers::initSwapChainFrameBuffers(SwapChain* swapChain, VkRenderPass renderPass) {
    
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();
    VkSampleCountFlagBits msaaSamples = context.getMsaaSamples();
    VkFormat colorFormat = swapChain->getSwapChainImageFormat();
    VkExtent2D extent = swapChain->getSwapChainExtent();
    std::vector<VkImageView> swapChainImageViews = swapChain->getSwapChainImageViews();
    
    VulkanUtil::createImage(
        extent.width, extent.height, 1, msaaSamples, colorFormat, 
        VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        colorImage, colorImageMemory);
    colorImageView = VulkanUtil::createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    VkFormat depthFormat = VulkanUtil::findDepthFormat();
    VulkanUtil::createImage(
        extent.width, extent.height, 1, msaaSamples, depthFormat, 
        VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        depthImage, depthImageMemory);
    depthImageView = VulkanUtil::createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    VkFormat resolveFormat = colorFormat;
    VulkanUtil::createImage(
        extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, resolveFormat, 
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        resolveImage, resolveImageMemory);

    resolveImageView = VulkanUtil::createImageView(resolveImage, resolveFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);


    framebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView, 4> attachments = {
            colorImageView,
            depthImageView,
            resolveImageView,
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}
