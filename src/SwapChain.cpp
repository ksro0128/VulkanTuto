#include "SwapChain.h"

std::unique_ptr<SwapChain> SwapChain::createSwapChain(GLFWwindow* window) {
    std::unique_ptr<SwapChain> swapChain = std::unique_ptr<SwapChain>(new SwapChain());
    swapChain->initSwapChain(window);
    return swapChain;
}


void SwapChain::cleanup() {
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}


void SwapChain::recreateSwapChain() {
    cleanup();
    createSwapChain();
    createImageViews();
}


void SwapChain::initSwapChain(GLFWwindow* window) {
    this->window = window;

    auto& context = VulkanContext::getContext();
    device = context.getDevice();
    physicalDevice = context.getPhysicalDevice();
    surface = context.getSurface();

    createSwapChain();
    createImageViews();
}


void SwapChain::createSwapChain() {
    // GPU와 surface가 지원하는 SwapChain 정보 불러오기
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    // 서피스 포맷 선택
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    // 프레젠테이션 모드 선택
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    // 스왑 범위 선택 (스왑 체인의 이미지 해상도 결정)
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // 스왑 체인에서 필요한 이미지 수 결정 (최소 이미지 수 + 1)
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    // 만약 필요한 이미지 수가 최댓값을 넘으면 clamp
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    
    // SwapChain 정보 생성
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    // 이미지 개수의 최솟값 설정 (최댓값은 아까 봤던 이미지 최댓값으로 들어감)
    createInfo.minImageCount = imageCount;
    // 이미지 정보 입력
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // 한 번의 렌더링에 n 개의 결과가 생긴다. (스테레오 3D, cubemap 이용시 여러 개 레이어 사용)
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ; // 기본 렌더링에만 사용하는 플래그 (만약 렌더링 후 2차 가공 필요시 다른 플래그 사용)

    // GPU가 지원하는 큐패밀리 목록 가져오기
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    // 큐패밀리 정보 등록
    if (indices.graphicsFamily != indices.presentFamily) {
        // [그래픽스 큐패밀리와 프레젠트 큐패밀리가 서로 다른 큐인 경우]
        // 하나의 이미지에 두 큐패밀리가 동시에 접근할 수 있게 하여 성능을 높인다.
        // (큐패밀리가 이미지에 접근할 때 다른 큐패밀리가 접근했는지 확인하는 절차 삭제)
        // 그러나 실제로 동시에 접근하면 안 되므로 순서를 프로그래머가 직접 조율해야 한다. 
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // 동시 접근 허용
        createInfo.queueFamilyIndexCount = 2;                     // 큐패밀리 개수 등록
        createInfo.pQueueFamilyIndices = queueFamilyIndices;      // 큐패밀리 인덱스 배열 등록
    } else {
        // [그래픽스 큐패밀리와 프레젠트 큐패밀리가 서로 같은 큐인 경우]
        // 어처피 1개의 큐패밀리만 존재하므로 큐패밀리의 이미지 독점을 허용한다.
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;  // 큐패밀리의 독점 접근 허용
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;  // 스왑 체인 이미지를 화면에 표시할때 적용되는 변환 등록
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // 렌더링 결과의 알파 값을 window에도 적용시킬지 설정 (현재는 알파블랜딩 없는 설정)
    createInfo.presentMode = presentMode; // 프레젠트 모드 설정
    createInfo.clipped = VK_TRUE; // 실제 컴퓨터 화면에 보이지 않는 부분을 렌더링 할 것인지 설정 (VK_TRUE = 렌더링 하지 않겠다)

    createInfo.oldSwapchain = VK_NULL_HANDLE; // 재활용할 이전 스왑체인 설정 (만약 설정한다면 새로운 할당을 하지 않고 가능한만큼 이전 스왑체인 리소스 재활용)

    /* 
    [스왑 체인 생성] 
    스왑 체인 생성시 이미지들도 설정대로 만들어지고, 
    만약 렌더링에 필요한 추가 이미지가 있으면 따로 만들어야 함 
    */
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    // 스왑 체인 이미지 개수 저장
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    // 스왑 체인 이미지 개수만큼 vector 초기화 
    swapChainImages.resize(imageCount);
    // 이미지 개수만큼 vector에 스왑 체인의 이미지 핸들 채우기 
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    // 스왑 체인의 이미지 포맷 저장
    swapChainImageFormat = surfaceFormat.format;
    // 스왑 체인의 이미지 크기 저장
    swapChainExtent = extent;
}


/*
[이미지 뷰 생성]
이미지 뷰란 VKImage에 대한 접근 방식을 정의하는 객체
GPU가 이미지를 읽을 때만 사용 (이미지를 텍스처로 쓰거나 후처리 하는 등)
GPU가 이미지에 쓰기 작업할 떄는 x
*/ 
void SwapChain::createImageViews() {
    // 이미지의 개수만큼 vector 초기화
    swapChainImageViews.resize(swapChainImages.size());

    // 이미지의 개수만큼 이미지뷰 생성
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = VulkanUtil::createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}


/*
GPU가 지원하는 큐패밀리 인덱스 가져오기
그래픽스 큐패밀리, 프레젠테이션 큐패밀리 인덱스를 저장
해당 큐패밀리가 없으면 optional 객체에 정보가 empty 상태
*/
QueueFamilyIndices SwapChain::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    // GPU가 지원하는 큐 패밀리 개수 가져오기
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    // GPU가 지원하는 큐 패밀리 리스트 가져오기
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // 그래픽 큐 패밀리 검색
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        // 그래픽 큐 패밀리 찾기 성공한 경우 indices에 값 생성
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        // GPU의 i 인덱스 큐 패밀리가 surface에서 프레젠테이션을 지원하는지 확인
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        // 프레젠테이션 큐 패밀리 등록
        if (presentSupport) {
            indices.presentFamily = i;
        }

        // 그래픽 큐 패밀리 찾은 경우 break
        if (indices.isComplete()) {
            break;
        }

        i++;
    }
    // 그래픽 큐 패밀리를 못 찾은 경우 값이 없는 채로 반환 됨
    return indices;
}


// GPU와 surface가 호환하는 SwapChain 정보를 반환
SwapChainSupportDetails SwapChain::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    // GPU와 surface가 호환할 수 있는 capability 정보 쿼리
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    // device에서 surface 객체를 지원하는 format이 존재하는지 확인 
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    // device에서 surface 객체를 지원하는 presentMode가 있는지 확인 
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}


/* 
지원하는 포맷중 선호하는 포맷 1개 반환
선호하는 포맷이 없을 시 가장 앞에 있는 포맷 반환
*/
VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        // 만약 선호하는 포맷이 존재할 경우 그 포맷을 반환
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    // 선호하는 포맷이 없는 경우 첫 번째 포맷 반환
    return availableFormats[0];
}


/*
지원하는 프레젠테이션 모드 중 선호하는 모드 선택
선호하는 모드가 없을 시 기본 값인 VK_PRESENT_MODE_FIFO_KHR 반환
*/
VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        // 선호하는 mode가 존재하면 해당 mode 반환 
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    // 선호하는 mode가 존재하지 않으면 기본 값인 VK_PRESENT_MODE_FIFO_KHR 반환
    return VK_PRESENT_MODE_FIFO_KHR;
}


// 스왑 체인 이미지의 해상도 결정
VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    // 만약 currentExtent의 width가 std::numeric_limits<uint32_t>::max()가 아니면, 시스템이 이미 권장하는 스왑체인 크기를 제공하는 것
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent; // 권장 사항 사용
    } else {
        // 그렇지 않은 경우, 창의 현재 프레임 버퍼 크기를 사용하여 스왑체인 크기를 결정
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        // width, height를 capabilites의 min, max 범위로 clamping
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}