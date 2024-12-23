#include "Pipeline.h"

std::unique_ptr<Pipeline> Pipeline::createPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout) {
    std::unique_ptr<Pipeline> pipeline = std::unique_ptr<Pipeline>(new Pipeline());
    pipeline->initPipeline(renderPass, descriptorSetLayout);
    return pipeline;
}


void Pipeline::cleanup() {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}


void Pipeline::initPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout) {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();
    VkSampleCountFlagBits msaaSamples = context.getMsaaSamples();

    // SPIR-V 파일 읽기
    std::vector<char> vertShaderCode = VulkanUtil::readFile("./spvs/shader.vert.spv");
    std::vector<char> fragShaderCode = VulkanUtil::readFile("./spvs/shader.frag.spv");

    // shader module 생성
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    /*
    shader stage 란?
    그래픽 파이프라인에서 사용할 셰이더 단계를 정의하는 구조체
    특정 셰이더 단계에서 사용할 셰이더 코드를 가지고 있음
    */ 

    // vertex shader stage 설정
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // 쉐이더 종류
    vertShaderStageInfo.module = vertShaderModule; // 쉐이더 모듈
    vertShaderStageInfo.pName = "main"; // 쉐이더 파일 내부에서 가장 먼저 시작 될 함수 이름 (엔트리 포인트)

    // fragment shader stage 설정
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // 쉐이더 종류
    fragShaderStageInfo.module = fragShaderModule; // 쉐이더 모듈
    fragShaderStageInfo.pName = "main"; // 쉐이더 파일 내부에서 가장 먼저 시작 될 함수 이름 (엔트리 포인트)

    // shader stage 모음
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};


    // [vertex 정보 설정]
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::getBindingDescription();												// 정점 바인딩 정보를 가진 구조체
    auto attributeDescriptions = Vertex::getAttributeDescriptions();										// 정점 속성 정보를 가진 구조체 배열

    vertexInputInfo.vertexBindingDescriptionCount = 1;														// 정점 바인딩 정보 개수
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());	// 정점 속성 정보 개수
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;										// 정점 바인딩 정보
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();							// 정점 속성 정보

    // [input assembly 설정] (그려질 primitive 설정)
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // primitive로 삼각형 설정
    inputAssembly.primitiveRestartEnable = VK_FALSE; // 인덱스 재시작 x

    /*
    [viewport와 scissor의 설정 값을 정의]
    viewport: 화면좌표로 매핑되어 정규화된 이미지 좌표를, viewport 크기에 맞는 픽셀 좌표로 변경 
                width, height는 (0, 0) ~ (width, height)로 depth는 (0.0f) ~ (1.0f)로 좌표 설정 
    scissor : 픽셀 좌표의 특정 영역에만 렌더링을 하도록 범위를 설정
                픽셀 좌표 내의 offset ~ extent 범위만 렌더링 진행 (나머지는 렌더링 x이므로 쓸데없는 계산 최소화)
    */ 
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1; // 사용할 뷰포트의 수
    viewportState.scissorCount = 1;  // 사용할 시저의수

    // [rasterizer 설정]
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;  				// VK_FALSE로 설정시 depth clamping이 적용되지 않아 0.0f ~ 1.0f 범위 밖의 프레그먼트는 삭제됨
    rasterizer.rasterizerDiscardEnable = VK_FALSE;  		// rasterization 진행 여부 결정, VK_TRUE시 렌더링 진행 x
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;  		// 다각형 그리는 방법 선택 (점만, 윤곽선만, 기본 값 등)
    rasterizer.lineWidth = 1.0f;							// 선의 굵기 설정 
    // rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;			// cull 모드 설정 (앞면 혹은 뒷면은 그리지 않는 설정 가능)
    rasterizer.cullMode = VK_CULL_MODE_NONE;				
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;	// 앞면의 기준 설정 (y축 반전에 의해 정점이 시계 반대방향으로 그려지므로 앞면을 시계 반대방향으로 설정)
    rasterizer.depthBiasEnable = VK_FALSE;					// depth에 bias를 설정하여 z-fighting 해결할 수 있음 (원근 투영시 멀어질 수록 z값의 차이가 미미해짐)
                                                            // VK_TRUE일 경우 추가 설정 필요

    // [멀티 샘플링 설정]
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_TRUE; 		// VK_TRUE: 프레그먼트 셰이더 단계(음영 계산)부터 샘플별로 계산 후 최종 결과 평균내서 사용 
                                                        // VK_FALSE: 테스트&블랜딩 단계부터 샘플별로 계산 후 최종 결과 평균내서 사용 (음영 계산은 동일한 값) 
    multisampling.minSampleShading = 0.2f; 				// 샘플 셰이딩의 최소 비율; 값이 1에 가까울수록 더 부드러워짐
    multisampling.rasterizationSamples = msaaSamples; 	// 픽셀당 샘플 개수 설정

    // [depth test]
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;				// 깊이 테스트 활성화 여부를 지정
    depthStencil.depthWriteEnable = VK_TRUE;			// 깊이 버퍼 쓰기 활성화 여부
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;	// 깊이 비교 연산 설정 (VK_COMPARE_OP_LESS: 현재 픽셀의 깊이가 더 작으면 통과)
    depthStencil.depthBoundsTestEnable = VK_FALSE;		// 깊이 범위 테스트 활성화 여부를 지정
    depthStencil.stencilTestEnable = VK_FALSE;			// 스텐실 테스트 활성화 여부를 지정

    // [블랜딩 설정]
    // attachment 별 블랜딩 설정 (블랜딩 + 프레임 버퍼 기록 설정)
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    // 프레임 버퍼에 RGBA 값 쓰기 가능 모드 설정 (설정 안 하면 색 수정 x)
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE; // 블랜드 기능 off (블랜드 기능 on 시 추가적인 설정 필요)
    // 파이프라인 전체 블랜딩 설정 (attachment 블랜딩 설정 추가)
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE; // 논리 연산 블랜딩 off (블랜딩 대신 논리적 연산을 통해 색을 조합하는 방법으로, 사용시 블렌딩 적용 x)
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // 논리 연산 없이 그냥 전체 복사 (논리 연산 블랜딩이 off면 안 쓰임)
    colorBlending.attachmentCount = 1; // attachment 별 블랜딩 설정 개수
    colorBlending.pAttachments = &colorBlendAttachment; // attachment 별 블랜딩 설정 배열
    // 블랜딩 연산에 사용하는 변수 값 4개 설정 (모든 attachment에 공통으로 사용)
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;


    // [파이프라인에서 런타임에 동적으로 상태를 변경할 state 설정]
    std::vector<VkDynamicState> dynamicStates = {
        // Viewport와 Scissor 를 동적 상태로 설정
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();


    // [파이프라인 레이아웃 생성]
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; 									// 디스크립터 셋 레이아웃 개수
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; 					// 디스크립투 셋 레이아웃

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    // [파이프라인 정보 생성]
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2; 								// vertex shader, fragment shader 2개 사용
    pipelineInfo.pStages = shaderStages; 						// vertex shader, fragment shader 총 2개의 stageinfo 입력
    pipelineInfo.pVertexInputState = &vertexInputInfo; 			// 정점 정보 입력
    pipelineInfo.pInputAssemblyState = &inputAssembly;			// primitive 정보 입력
    pipelineInfo.pViewportState = &viewportState;				// viewport, scissor 정보 입력
    pipelineInfo.pRasterizationState = &rasterizer;				// 레스터라이저 설정 입력
    pipelineInfo.pMultisampleState = &multisampling;			// multisampling 설정 입력
    pipelineInfo.pDepthStencilState = &depthStencil;			// depth-stencil 설정
    pipelineInfo.pColorBlendState = &colorBlending;				// 블랜딩 설정 입력
    pipelineInfo.pDynamicState = &dynamicState;					// 동적으로 변경할 상태 입력
    pipelineInfo.layout = pipelineLayout;						// 파이프라인 레이아웃 설정 입력
    pipelineInfo.renderPass = renderPass;						// 렌더패스 입력
    pipelineInfo.subpass = 0;									// 렌더패스 내 서브패스의 인덱스
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;			// 상속을 위한 기존 파이프라인 핸들
    pipelineInfo.basePipelineIndex = -1; 						// Optional (상속을 위한 기존 파이프라인 인덱스)	

    // [파이프라인 객체 생성]
    // 두 번째 매개변수는 상속할 파이프라인
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    // 쉐이더 모듈 더 안 쓰므로 제거
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

/*
매개변수로 받은 쉐이더 파일을 shader module로 만들어 줌
shader module은 쉐이더 파일을 객체화 한 것임
*/ 
VkShaderModule Pipeline::createShaderModule(const std::vector<char>& code) {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();									// 코드 길이 입력
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());	// 코드 내용 입력

    // 쉐이더 모듈 생성
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}
