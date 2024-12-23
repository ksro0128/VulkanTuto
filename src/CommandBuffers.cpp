#include "CommandBuffers.h"

std::unique_ptr<CommandBuffers> CommandBuffers::createCommandBuffers() {
    std::unique_ptr<CommandBuffers> commandBuffers = std::unique_ptr<CommandBuffers>(new CommandBuffers());
    commandBuffers->initCommandBuffers();
    return commandBuffers;
}


void CommandBuffers::cleanup() {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    vkFreeCommandBuffers(device, context.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
}


/*
[커맨드 버퍼 생성]
커맨드 버퍼에 GPU에서 실행할 작업을 전부 기록한뒤 제출한다.
GPU는 해당 커맨드 버퍼의 작업을 알아서 실행하고, CPU는 다른 일을 할 수 있게 된다. (병렬 처리)
*/
void CommandBuffers::initCommandBuffers() {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();
    VkCommandPool commandPool = context.getCommandPool();

    // 동시에 처리할 프레임 버퍼 수만큼 커맨드 버퍼 생성
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    // 커맨드 버퍼 설정값 준비
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool; 								// 커맨드 풀 등록
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;					// 큐에 직접 제출할 수 있는 커맨드 버퍼 설정
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size(); 	// 할당할 커맨드 버퍼의 개수

    // 커맨드 버퍼 할당
    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
