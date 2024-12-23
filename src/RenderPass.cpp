#include "RenderPass.h"

std::unique_ptr<RenderPass> RenderPass::createRenderPass(VkFormat swapChainImageFormat) {
    std::unique_ptr<RenderPass> renderPass = std::unique_ptr<RenderPass>(new RenderPass());
    renderPass->initRenderPass(swapChainImageFormat);
    return renderPass;
}


void RenderPass::cleanup(){
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    vkDestroyRenderPass(device, renderPass, nullptr);
}


void RenderPass::initRenderPass(VkFormat swapChainImageFormat) {
    // [attachment 설정]
    // FrameBuffer의 attachment에 어떤 정보를 어떻게 기록할지 정하는 객체
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();
    VkSampleCountFlagBits msaaSamples = context.getMsaaSamples();

    // 멀티 샘플링 color attachment 설정
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;		 					// 이미지 포맷 (스왑 체인과 일치 시킴)
    colorAttachment.samples = msaaSamples;			 						// 샘플 개수 (멀티 샘플링을 위한 값 사용)
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;					// 렌더링 전 버퍼 클리어 (렌더링 시작 시 기존 attachment의 데이터 처리 방법)
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; 				// 렌더링 결과 저장 (렌더링 후 attachment를 메모리에 저장하는 방법 결정)
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; 		// 이전 데이터 무시 (스텐실 버퍼의 loadOp)
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; 		// 저장 x (스텐실 버퍼의 storeOp)
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; 				// 초기 레이아웃 설정을 UNDEFINED로 설정 (초기 데이터 가공을 하지 않기 때문에 가장 빠름)
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // color attachment 레이아웃 설정 

    // depth attachment 설정
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VulkanUtil::findDepthFormat();
    depthAttachment.samples = msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;						// 초기 레이아웃 설정을 UNDEFINED로 설정
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // 최종 레이아웃 depth-stencil buffer로 사용

    // resolve attachment 설정
    // 멀티 샘플링 attachment를 단일 샘플링 attachment로 전환
    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // subpass가 attachment 설정 어떤 것을 어떻게 참조할지 정의
    // color attachment
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; 										// 특정 attachment 설정의 index
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// attachment 설정을 subpass 내에서
                                                                            // 어떤 layout으로 쓸지 결정 (현재는 color attachment로 사용하는 설정)
    // depth attachment
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // resolve attachment
    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // [subpass 정의]
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1; 										// attachment 설정 개수 등록
    subpass.pColorAttachments = &colorAttachmentRef;						// color attachment 등록
    subpass.pDepthStencilAttachment = &depthAttachmentRef;					// depth attachment 등록
    subpass.pResolveAttachments = &colorAttachmentResolveRef;				// resolve attachment 등록

    // [subpass 종속성 설정]
    // 렌더패스 외부 작업(srcSubpass)과 0번 서브패스(dstSubpass) 간의 동기화 설정.
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;	// 렌더패스 외부 작업(이전 프레임 처리 또는 렌더패스 외부의 GPU 작업)
    dependency.dstSubpass = 0;					 	// 첫 번째 서브패스(0번 서브패스)에 종속
    // srcStageMask: 동기화를 기다릴 렌더패스 외부 작업의 파이프라인 단계
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;	// 색상 첨부물 출력 단계 | 프래그먼트 테스트의 최종 단계
    // srcAccessMask: 렌더패스 외부 작업에서 보장해야 할 메모리 접근 권한
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;												// 깊이/스텐실 첨부물 쓰기 권한
    // dstStageMask: 0번 서브패스에서 동기화를 기다릴 파이프라인 단계
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;	// 색상 첨부물 출력 단계 | 프래그먼트 테스트의 초기 단계
    // dstAccessMask: 0번 서브패스에서 필요한 메모리 접근 권한
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;			// 색상 첨부물 쓰기 권한 | 깊이/스텐실 첨부물 쓰기 권한

    // [렌더 패스 정의]
    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); // attachment 설정 개수 등록
    renderPassInfo.pAttachments = attachments.data();							// attachment 설정 등록
    renderPassInfo.subpassCount = 1;											// subpass 개수 등록
    renderPassInfo.pSubpasses = &subpass;										// subpass 등록
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    // [렌더 패스 생성]
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}
