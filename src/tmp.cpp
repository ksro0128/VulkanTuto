#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define GLM_FORCE_RADIANS
// GLM에서는 보통 -1.0 ~ 1.0 범위로 원근 투영 행렬을 사용하므로 0.0 ~ 1.0 범위로 한다는 설정 적용
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>



// 동시에 처리할 최대 프레임 수
const int MAX_FRAMES_IN_FLIGHT = 2;

// 검증 레이어 설정
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

// 스왑 체인 확장
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// 디버그 모드시 검증 레이어 사용
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

/*
	디버그 메신저 객체 생성 함수
	(확장 함수는 자동으로 로드 되지 않으므로 동적으로 가져와야 한다.)
*/
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
										const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	// vkCreateDebugUtilsMessengerEXT 함수의 주소를 vkGetInstanceProcAddr를 통해 동적으로 가져옴
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

/*
	디버그 메신저 객체 파괴 함수
	(확장 함수는 자동으로 로드 되지 않으므로 동적으로 가져와야 한다.)
*/
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

// 큐 패밀리 인덱스 관리 구조체
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

// GPU와 surface가 지원하는 SwapChain 지원 세부 정보 구조체
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	// 정점 데이터가 전달되는 방법을 알려주는 구조체 반환하는 함수
	static VkVertexInputBindingDescription getBindingDescription() {
		// 파이프라인에 정점 데이터가 전달되는 방법을 알려주는 구조체
		VkVertexInputBindingDescription bindingDescription{};		
		bindingDescription.binding = 0;								// 버텍스 바인딩 포인트 (현재 0번에 vertex 정보 바인딩)
		bindingDescription.stride = sizeof(Vertex);					// 버텍스 1개 단위의 정보 크기
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // 정점 데이터 처리 방법
																	// 1. VK_VERTEX_INPUT_RATE_VERTEX : 정점별로 데이터 처리
																	// 2. VK_VERTEX_INPUT_RATE_INSTANCE : 인스턴스별로 데이터 처리
		return bindingDescription;
	}

	// 정점 속성별 데이터 형식과 위치를 지정하는 구조체 반환하는 함수
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		// 정점 속성의 데이터 형식과 위치를 지정하는 구조체
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		// pos 속성 정보 입력
		attributeDescriptions[0].binding = 0;							// 버텍스 버퍼의 바인딩 포인트
		attributeDescriptions[0].location = 0;							// 버텍스 셰이더의 어떤 location에 대응되는지 지정
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;	// 저장되는 데이터 형식 (VK_FORMAT_R32G32B32_SFLOAT = vec3)
		attributeDescriptions[0].offset = offsetof(Vertex, pos);		// 버텍스 구조체에서 해당 속성이 시작되는 위치

		// color 속성 정보 입력
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		// texCoord 속성 정보 입력
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class Window {
public:
	static std::unique_ptr<Window> createWindow() {
		std::unique_ptr<Window> window = std::unique_ptr<Window>(new Window());
		window->initWindow();
		return window;
	}
	~Window() {
	}
	GLFWwindow* getWindow() {
		return window;
	}
	void cleanup() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}
private:
	GLFWwindow* window;

	Window() {}
	void initWindow() {
		glfwInit();

		// glfw로 창관리만 하고 렌더링 API는 안 쓴다는 설정
		// 렌더링 및 그래픽 처리는 외부 API인 Vulkan으로 함
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);
		// window에 현재 App 객체를 바인딩
		glfwSetWindowUserPointer(window, this);
		// 프레임버퍼 사이즈 변경 콜백 함수 등록
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		// window에 바인딩된 객체 호출 및 framebufferResized = true 설정
		
	}
};

class VulkanContext {
public:
	static VulkanContext& getContext() {
		static VulkanContext context;
		return context;
	}

	void initContext(GLFWwindow* window) {
		createInstance();
		setupDebugMessenger();
		createSurface(window);
		pickPhysicalDevice();
		createLogicalDevice();
		createCommandPool();
		createDescriptorPool();
	}

	~VulkanContext() {}

	void cleanup() {
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyCommandPool(device, commandPool, nullptr);
		vkDestroyDevice(device, nullptr);
		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
	}

	VkInstance getInstance() { return instance; }
	VkDebugUtilsMessengerEXT getDebugMessenger() { return debugMessenger; }
	VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
	VkDevice getDevice() { return device; }
	VkQueue getGraphicsQueue() { return graphicsQueue; }
	VkQueue getPresentQueue() { return presentQueue; }
	VkCommandPool getCommandPool() { return commandPool; }
	VkSurfaceKHR getSurface() { return surface; }
	VkSampleCountFlagBits getMsaaSamples() { return msaaSamples; }
	VkDescriptorPool getDescriptorPool() { return descriptorPool; }


	void createSurface(GLFWwindow* window) {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

private:
	VulkanContext() { }
	VulkanContext(VulkanContext const&) = delete;
	void operator=(VulkanContext const&) = delete;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkDevice device;

	VkCommandPool commandPool;
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkDescriptorPool descriptorPool;

	void createInstance() {
		// 디버그 모드에서 검증 레이어 적용 불가능시 예외 발생
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		// 애플리케이션 정보를 담은 구조체
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// 인스턴스 생성을 위한 정보를 담은 구조체
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		std::vector<const char*> extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
	
		// 디버깅 메시지 객체 생성을 위한 정보 구조체
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

		if (enableValidationLayers) {
			// 디버그 모드시 구조체에 검증 레이어 포함
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
			// 인스턴스 생성 및 파괴시에도 검증 가능
			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
		} else {
			// 디버그 모드 아닐 시 검증 레이어 x
			createInfo.enabledLayerCount = 0;		
			createInfo.pNext = nullptr;
		}

		// 인스턴스 생성
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	// 검증 레이어가 사용 가능한 레이어 목록에 있는지 확인
	bool checkValidationLayerSupport() {
		// Vulkan 인스턴스에서 사용 가능한 레이어들 목록 생성
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// 필요한 검증 레이어들이 사용 가능 레이어에 포함 되어있는지 확인
		for (const char* layerName : validationLayers) {
			bool layerFound = false;
			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					// 포함 확인!
					layerFound = true;
					break;
				}
			}
			if (!layerFound) {
				return false;  // 필요한 레이어가 없다면 false 반환
			}
		}
		return true;  // 모든 레이어가 지원되면 true 반환
	}

	/*
	GLFW 라이브러리에서 Vulkan 인스턴스를 생성할 때 필요한 인스턴스 확장 목록을 반환
	(디버깅 모드시 메시지 콜백 확장 추가)
	*/
	std::vector<const char*> getRequiredExtensions() {
		// 필요한 확장 목록 가져오기
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		// 디버깅 모드이면 VK_EXT_debug_utils 확장 추가 (메세지 콜백 확장)
		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		
		return extensions;
	}

	// 디버그 메신저 객체 생성
	void setupDebugMessenger() {
		// 디버그 모드 아니면 return
		if (!enableValidationLayers) return;

		// VkDebugUtilsMessengerCreateInfoEXT 구조체 생성
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		// 디버그 메시지 객체 생성
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	//vkDebugUtilsMessengerCreateInfoEXT 구조체 내부를 채워주는 함수
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
									VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
									VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
								VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
								VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	// 적절한 GPU 고르는 함수
	void pickPhysicalDevice() {
		// GPU 장치 목록 불러오기
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
  			  throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		// 적합한 GPU 탐색
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				msaaSamples = getMaxUsableSampleCount();
				break;
			}
		}

		// 적합한 GPU가 발견되지 않은 경우 에러 발생
		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	// GPU와 소통할 인터페이스인 Logical device 생성
	void createLogicalDevice() {
		// 그래픽 큐 패밀리의 인덱스를 가져옴
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		// 큐 패밀리의 인덱스들을 set으로 래핑
		std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		// 큐 생성을 위한 정보 설정 
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		// 큐 우선순위 0.0f ~ 1.0f 로 표현
		float queuePriority = 1.0f;
		// 큐 패밀리 별로 정보 생성
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// 사용할 장치 기능이 포함된 구조체
		// vkGetPhysicalDeviceFeatures 함수로 디바이스에서 설정 가능한
		// 장치 기능 목록을 확인할 수 있음
		// 일단 지금은 VK_FALSE로 전부 등록함
		VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;		// 이방성 필터링 사용 설정
		deviceFeatures.sampleRateShading = VK_TRUE; 	// 디바이스에 샘플 셰이딩 기능 활성화

		// 논리적 장치 생성을 위한 정보 등록
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		// 확장 설정
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		
		// 구버전 호환을 위해 디버그 모드일 경우
		// 검증 레이어를 포함 시키지만, 현대 시스템에서는 논리적 장치의 레이어를 안 씀
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		// 논리적 장치 생성
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
 		   throw std::runtime_error("failed to create logical device!");
		}

		// 큐 핸들 가져오기
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	/*
		[커맨드 풀 생성]
		커맨드 풀이란?
		1. 커맨드 버퍼들을 관리한다.
		2. 큐 패밀리당 1개의 커맨드 풀이 필요하다.
	*/
	void createCommandPool() {
		// 큐 패밀리 인덱스 가져오기
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; 		// 커맨드 버퍼를 개별적으로 재설정할 수 있도록 설정 
																				// (이게 아니면 커맨드 풀의 모든 커맨드 버퍼 설정이 한 번에 이루어짐)
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(); 	// 그래픽스 큐 인덱스 등록 (대응시킬 큐 패밀리 등록)

		// 커맨드 풀 생성
		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}


	// 그래픽, 프레젠테이션 큐 패밀리가 존재하는지, GPU와 surface가 호환하는 SwapChain이 존재하는지 검사
	bool isDeviceSuitable(VkPhysicalDevice device) {
		// 큐 패밀리 확인
		QueueFamilyIndices indices = findQueueFamilies(device);
		
		// 스왑 체인 확장을 지원하는지 확인
		bool extensionsSupported = checkDeviceExtensionSupport(device);
		bool swapChainAdequate = false;

		// 스왑 체인 확장이 존재하는 경우
		if (extensionsSupported) {
			// 물리 디바이스와 surface가 호환하는 SwapChain 정보를 가져옴
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			// GPU와 surface가 지원하는 format과 presentMode가 존재하면 통과
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		// GPU 에서 이방성 필터링을 지원하는지 확인
        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	// GPU와 surface가 호환하는 SwapChain 정보를 반환
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
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

	// GPU 에서 지원하는 최대 샘플 개수 반환
	VkSampleCountFlagBits getMaxUsableSampleCount() {
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		// GPU가 지원하는 color 샘플링 개수와 depth 샘플링 개수의 공통 분모를 찾음
		VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

		// 가장 높은 샘플링 개수부터 확인하면서 반환
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}

	// 디바이스가 지원하는 확장 중 
	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		// 스왑 체인 확장이 존재하는지 확인
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
		for (const auto& extension : availableExtensions) {
			// 지원 가능한 확장들 목록을 순회하며 제거
			requiredExtensions.erase(extension.extensionName);
		}

		// 만약 기존에 있던 확장이 제거되면 true
		// 기존에 있던 확장이 그대로면 지원을 안 하는 것이므로 false
		return requiredExtensions.empty();
	}

	/*
	GPU가 지원하는 큐패밀리 인덱스 가져오기
	그래픽스 큐패밀리, 프레젠테이션 큐패밀리 인덱스를 저장
	해당 큐패밀리가 없으면 optional 객체에 정보가 empty 상태
	*/
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
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

	// 디스크립터 풀 생성
	void createDescriptorPool() {
		size_t MAX_OBJECTS = 1000;

		// 디스크립터 풀의 타입별 디스크립터 개수를 설정하는 구조체
        std::array<VkDescriptorPoolSize, 4> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;							// 유니폼 버퍼 설정
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * MAX_OBJECTS);		// 유니폼 버퍼 디스크립터 최대 개수 설정
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;					// 샘플러 설정
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * MAX_OBJECTS);		// 샘플러 디스크립터 최대 개수 설정
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * MAX_OBJECTS);
		poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * MAX_OBJECTS);



		// 디스크립터 풀을 생성할 때 필요한 설정 정보를 담는 구조체
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());			// 디스크립터 poolSize 구조체 개수
        poolInfo.pPoolSizes = poolSizes.data();										// 디스크립터 poolSize 구조체 배열
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * MAX_OBJECTS);				// 풀에 존재할 수 있는 총 디스크립터 셋 개수

		// 디스크립터 풀 생성
		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}



	// 디버그 메시지 콜백 함수
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
														VkDebugUtilsMessageTypeFlagsEXT messageType,
														const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
														void* pUserData) {
		// 메시지 내용만 출력
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		// VK_TRUE 반환시 프로그램 종료됨
		return VK_FALSE;
	}

};


class VulkanUtil {
public:
	static void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
		
		auto& context = VulkanContext::getContext();
		auto device = context.getDevice();

		// 이미지 객체를 만드는데 사용되는 구조체
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;					// 이미지의 차원을 설정
		imageInfo.extent.width = width;							// 이미지의 너비 지정
		imageInfo.extent.height = height;						// 이미지의 높이 지정 
		imageInfo.extent.depth = 1;								// 이미지의 깊이 지정 (2D 이미지의 경우 depth는 1로 지정해야 함)
		imageInfo.mipLevels = mipLevels;						// 생성할 mipLevel의 개수 지정
		imageInfo.arrayLayers = 1;								// 생성할 이미지 레이어 수 (큐브맵의 경우 6개 생성)
		imageInfo.format = format;								// 이미지의 포맷을 지정하며, 채널 구성과 각 채널의 비트 수를 정의
		imageInfo.tiling = tiling;								// 이미지를 GPU 메모리에 배치할 때 메모리 레이아웃을 결정하는 설정 (CPU에서도 접근 가능하게 할꺼냐, GPU에만 접근 가능하게 최적화 할거냐 결정) 
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;	// 이미지 초기 레이아웃 설정 (이미지가 메모리에 배치될 때 초기 상태를 정의)
		imageInfo.usage = usage;								// 이미지의 사용 용도 결정
		imageInfo.samples = numSamples;							// 멀티 샘플링을 위한 샘플 개수
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		// 이미지의 큐 공유 모드 설정 (VK_SHARING_MODE_EXCLUSIVE: 한 번에 하나의 큐 패밀리에서만 접근 가능한 단일 큐 모드)

		// 이미지 객체 생성
		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		// 이미지에 필요한 메모리 요구 사항을 조회 
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		// 메모리 할당을 위한 구조체
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;											// 메모리 크기 설정
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);		// 메모리 유형과 속성 설정

		// 이미지를 위한 메모리 할당
		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		// 이미지에 할당한 메모리 바인딩
		vkBindImageMemory(device, image, imageMemory, 0);
	}

	/*
		GPU와 buffer가 호환되는 메모리 유형중 properties에 해당하는 속성들을 갖는 메모리 유형 찾기
	*/
	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		auto& context = VulkanContext::getContext();
		auto physicalDevice = context.getPhysicalDevice();
		
		// GPU에서 사용한 메모리 유형을 가져온다.
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			// typeFilter & (1 << i) : GPU의 메모리 유형중 버퍼와 호환되는 것인지 판단
			// memProperties.memoryTypes[i].propertyFlags & properties : GPU 메모리 유형의 속성이 properties와 일치하는지 판단
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				// 해당 메모리 유형 반환
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	// 이미지 뷰 생성
	static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
		auto& context = VulkanContext::getContext();
		auto device = context.getDevice();
		
		// 이미지 뷰 정보 생성
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;													// 이미지 핸들
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;								// 이미지 타입
		viewInfo.format = format;												// 이미지 포맷
		viewInfo.subresourceRange.aspectMask = aspectFlags;  					// 이미지 형식 결정 (color / depth / stencil 등)
		viewInfo.subresourceRange.baseMipLevel = 0;                          	// 렌더링할 mipmap 단계 설정
		viewInfo.subresourceRange.levelCount = mipLevels;                       // baseMipLevel 기준으로 몇 개의 MipLevel을 더 사용할지 설정 (실제 mipmap 만드는 건 따로 해줘야함)
		viewInfo.subresourceRange.baseArrayLayer = 0;                        	// ImageView가 참조하는 이미지 레이어의 시작 위치 정의
		viewInfo.subresourceRange.layerCount = 1;                            	// 스왑 체인에서 설정한 이미지 레이어 개수

		// 이미지 뷰 생성
		VkImageView imageView;
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image view!");
		}

		return imageView;
	}

	// depth image의 format 설정
	static VkFormat findDepthFormat() {
		return findSupportedFormat(
			{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	// Vulkan의 특정 format에 대해 GPU가 tiling의 features를 지원하는지 확인
	static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
		auto& context = VulkanContext::getContext();
		VkPhysicalDevice physicalDevice = context.getPhysicalDevice();

		// format 들에 대해 GPU가 tiling과 features를 지원하는지 확인
		for (VkFormat format : candidates) {
			// GPU가 format에 대해 지원하는 특성 가져오는 함수
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			// GPU가 지원하는 특성과 비교
			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {				// VK_IMAGE_TILING_LINEAR의 특성 비교
				return format;
			} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {		// VK_IMAGE_TILING_OPTIMAL의 특성 비교
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format!");
	}

	// shader 파일인 SPIR-V 파일을 바이너리 형태로 읽어오는 함수
	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t) file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

};


class Buffer {
public:
	virtual ~Buffer() {
	}
	virtual void cleanup() = 0;
protected:
	VkBuffer m_buffer;
	VkDeviceMemory m_bufferMemory;

	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkCommandPool m_commandPool;
	VkQueue m_graphicsQueue;

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
		// 버퍼 객체를 생성하기 위한 구조체 (GPU 메모리에 데이터 저장 공간을 할당하는 데 필요한 설정을 정의)
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;										// 버퍼의 크기 지정
		bufferInfo.usage = usage;									// 버퍼의 용도 지정
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;			// 버퍼를 하나의 큐 패밀리에서 쓸지, 여러 큐 패밀리에서 공유할지 설정 (현재 단일 큐 패밀리에서 사용하도록 설정)
																	// 여러 큐 패밀리에서 공유하는 모드 사용시 추가 설정 필요
		// [버퍼 생성]
		// 버퍼를 생성하지만 할당은 안되어있는 상태로 만들어짐       
		if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		// [버퍼에 메모리 할당]
		// 메모리 할당 요구사항 조회
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

		// 메모리 할당을 위한 구조체
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;	// 할당할 메모리 크기
		// 메모리 요구사항 설정 (GPU 메모리 유형 중 buffer와 호환되고 properties 속성들과 일치하는 것 찾아 저장)
		// 메모리 유형 - GPU 메모리는 구역마다 유형이 다르다. (memoryTypeBits는 buffer가 호환되는 GPU의 메모리 유형이 전부 담겨있음)
		// 메모리 유형의 속성 - 메모리 유형마다 특성을 가지고 있음
		allocInfo.memoryTypeIndex = VulkanUtil::findMemoryType(memRequirements.memoryTypeBits, properties);

		// 버퍼 메모리 할당
		if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		// 버퍼 객체에 할당된 메모리를 바인딩 (4번째 매개변수는 할당할 메모리의 offset)
		vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
	}



	// srcBuffer 에서 dstBuffer 로 데이터 복사
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{}; 	// 복사할 버퍼 영역을 지정 (크기, src 와 dst의 시작 offset 등)
		copyRegion.size = size;		// 복사할 버퍼 크기 설정
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion); // 커맨드 버퍼에 복사 명령 기록

        endSingleTimeCommands(commandBuffer);		
	}

	// 한 번만 실행할 커맨드 버퍼 생성 및 기록 시작
	VkCommandBuffer beginSingleTimeCommands() {
		// 커맨드 버퍼 할당을 위한 구조체 
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;		// PRIMARY LEVEL 등록 (해당 커맨드 버퍼가 큐에 단독으로 제출될 수 있음)
		allocInfo.commandPool = m_commandPool;					// 커맨드 풀 지정
		allocInfo.commandBufferCount = 1;						// 커맨드 버퍼 개수 지정

		// 커맨드 버퍼 생성
		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

		// 커맨드 버퍼 기록을 위한 정보 객체
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // 커맨드 버퍼를 1번만 제출

		// GPU에 필요한 작업을 모두 커맨드 버퍼에 기록하기 시작
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	// 한 번만 실행할 커맨드 버퍼 기록 중지 및 큐에 커맨드 버퍼 제출
	void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
		// 커맨드 버퍼 기록 중지
		vkEndCommandBuffer(commandBuffer);

		// 복사 커맨드 버퍼 제출 정보 객체 생성
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;								// 커맨드 버퍼 개수
		submitInfo.pCommandBuffers = &commandBuffer;					// 커맨드 버퍼 등록

		vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);	// 커맨드 버퍼 큐에 제출
		vkQueueWaitIdle(m_graphicsQueue);									// 그래픽스 큐 작업 종료 대기

		vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);	// 커맨드 버퍼 제거
	}	
};

class VertexBuffer : public Buffer {
public:
	static std::unique_ptr<VertexBuffer> createVertexBuffer(std::vector<Vertex>& vertices) {
		std::unique_ptr<VertexBuffer> vertexBuffer = std::unique_ptr<VertexBuffer>(new VertexBuffer());
		vertexBuffer->initVertexBuffer(vertices);
		return vertexBuffer;
	}

	void bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = { m_buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void cleanup() {
		vkDestroyBuffer(m_device, m_buffer, nullptr);
		vkFreeMemory(m_device, m_bufferMemory, nullptr);
	}

	~VertexBuffer() {}

private:

	void initVertexBuffer(std::vector<Vertex>& vertices) {
		auto &context = VulkanContext::getContext();
		m_device = context.getDevice();
		m_physicalDevice = context.getPhysicalDevice();
		m_commandPool = context.getCommandPool();
		m_graphicsQueue = context.getGraphicsQueue();

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);

		vkUnmapMemory(m_device, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_buffer, m_bufferMemory);
		copyBuffer(stagingBuffer, m_buffer, bufferSize);

		vkDestroyBuffer(m_device, stagingBuffer, nullptr);
		vkFreeMemory(m_device, stagingBufferMemory, nullptr);
	}
};

class IndexBuffer : public Buffer {
public:
	static std::unique_ptr<IndexBuffer> createIndexBuffer(std::vector<uint32_t>& indices) {
		std::unique_ptr<IndexBuffer> indexBuffer = std::unique_ptr<IndexBuffer>(new IndexBuffer());
		indexBuffer->initIndexBuffer(indices);
		return indexBuffer;
	}

	void bind(VkCommandBuffer commandBuffer) {
		vkCmdBindIndexBuffer(commandBuffer, m_buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void cleanup() {
		vkDestroyBuffer(m_device, m_buffer, nullptr);
		vkFreeMemory(m_device, m_bufferMemory, nullptr);
	}

	uint32_t getIndexCount() {
		return m_indexCount;
	}

	~IndexBuffer() {}

private:
	uint32_t m_indexCount;

	void initIndexBuffer(std::vector<uint32_t>& indices) {
		auto &context = VulkanContext::getContext();
		m_device = context.getDevice();
		m_physicalDevice = context.getPhysicalDevice();
		m_commandPool = context.getCommandPool();
		m_graphicsQueue = context.getGraphicsQueue();

		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
		m_indexCount = static_cast<uint32_t>(indices.size());

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_device, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_buffer, m_bufferMemory);
		copyBuffer(stagingBuffer, m_buffer, bufferSize);

		vkDestroyBuffer(m_device, stagingBuffer, nullptr);
		vkFreeMemory(m_device, stagingBufferMemory, nullptr);
	}
};


class ImageBuffer : public Buffer {
public:
	static std::unique_ptr<ImageBuffer> createImageBuffer(std::string path) {
		std::unique_ptr<ImageBuffer> imageBuffer = std::unique_ptr<ImageBuffer>(new ImageBuffer());
		imageBuffer->initImageBuffer(path);
		return imageBuffer;
	}

	void cleanup() {
		vkDestroyImage(m_device, textureImage, nullptr);
		vkFreeMemory(m_device, textureImageMemory, nullptr);
	}

	~ImageBuffer() {}

	uint32_t getMipLevels() { return mipLevels; }
	VkImage getImage() { return textureImage; }
	VkDeviceMemory getImageMemory() { return textureImageMemory; }

private:
	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;

	void initImageBuffer(std::string path) {
		auto &context = VulkanContext::getContext();
		m_device = context.getDevice();
		m_physicalDevice = context.getPhysicalDevice();
		m_commandPool = context.getCommandPool();
		m_graphicsQueue = context.getGraphicsQueue();

		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_device, stagingBufferMemory);

		stbi_image_free(pixels);
		VulkanUtil::createImage(texWidth, texHeight, mipLevels, 
		VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, 
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
		copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

		vkDestroyBuffer(m_device, stagingBuffer, nullptr);
		vkFreeMemory(m_device, stagingBufferMemory, nullptr);

		generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
	}


	// 이미지 레이아웃, 접근 권한을 변경할 수 있는 베리어를 커맨드 버퍼에 기록
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();			// 커맨드 버퍼 생성 및 기록 시작

		// 베리어 생성을 위한 구조체
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;										// src 단계까지의 이미지 레이아웃
		barrier.newLayout = newLayout;										// src 단계 이후 적용시킬 새로운 이미지 레이아웃
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;				// 베리어 전환 적용 후 리소스 소유권을 넘겨줄 src 큐 패밀리 (현재는 동일 큐 패밀리에서 실행)
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;				// 리소스 소유권을 받을 dst 큐 패밀리로 dst 큐패밀리에는 큐 전체에 동기화가 적용 (현재는 동일 큐 패밀리에서 실행)
		barrier.image = image;												// 배리어 적용할 이미지 객체
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// 전환 작업의 적용 대상을 color bit 으로 설정
		barrier.subresourceRange.baseMipLevel = 0;							// 전환 작업을 시작할 miplevel
		barrier.subresourceRange.levelCount = mipLevels;					// 전환 작업을 적용할 miplevel의 개수
		barrier.subresourceRange.baseArrayLayer = 0;						// 전환 작업을 시작할 레이어 인덱스
		barrier.subresourceRange.layerCount = 1;							// 전환 작업을 적용할 레이어 개수

		VkPipelineStageFlags sourceStage;									// 파이프라인의 sourceStage 단계가 끝나면 배리어 전환 실행 
		VkPipelineStageFlags destinationStage;								// destinationStage 단계는 배리어 전환이 끝날때까지 대기

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			// 이미지 복사 전에 이미지 레이아웃, 접근 권한 변경
			barrier.srcAccessMask = 0;									// 접근 제한 x
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;		// 쓰기 권한 필요

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;			// Vulkan의 파이프라인에서 가장 상단에 위치한 첫 번째 단계로, 어떠한 작업도 진행되지 않은 상태
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;			// 데이터 복사 단계
		} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			// 이미지 복사가 완료되고 읽기를 수행하기 위해 Fragment shader 작업 대기
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;		// 쓰기 권한 필요
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;			// 읽기 권한 필요

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;				// 데이터 복사 단계
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;	// Fragment shader 단계
		} else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		// 베리어를 커맨드 버퍼에 기록
		vkCmdPipelineBarrier(
			commandBuffer,						// 베리어를 기록할 커맨드 버퍼
			sourceStage, destinationStage,		// sourceStage 단계가 끝나면 베리어 작업 시작, 베리어 작업이 끝나기 전에 destinationStage에 돌입한 다른 작업들 모두 대기
			0,									// 의존성 플래그
			0, nullptr,							// 메모리 베리어 (개수 + 베리어 포인터)
			0, nullptr,							// 버퍼 베리어   (개수 + 베리어 포인터)
			1, &barrier							// 이미지 베리어 (개수 + 베리어 포인터)
		);

		endSingleTimeCommands(commandBuffer);	// 커맨드 버퍼 기록 종료
	}

	// 커맨드 버퍼 제출을 통해 버퍼 -> 이미지 데이터 복사 
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		// 커맨드 버퍼 생성 및 기록 시작
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		// 버퍼 -> 이미지 복사를 위한 정보
		VkBufferImageCopy region{};
		region.bufferOffset = 0;											// 복사할 버퍼의 시작 위치 offset
		region.bufferRowLength = 0;											// 저장될 공간의 row 당 픽셀 수 (0으로 하면 이미지 너비에 자동으로 맞춰진다.)
		region.bufferImageHeight = 0;										// 저장될 공간의 col 당 픽셀 수 (0으로 하면 이미지 높이에 자동으로 맞춰진다.)
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;		// 이미지의 데이터 타입 (현재는 컬러값을 복사)
		region.imageSubresource.mipLevel = 0;								// 이미지의 miplevel 설정
		region.imageSubresource.baseArrayLayer = 0;							// 이미지의 시작 layer 설정 (cubemap과 같은 경우 여러 레이어 존재)
		region.imageSubresource.layerCount = 1;								// 이미지 layer 개수
		region.imageOffset = {0, 0, 0};										// 이미지의 저장할 시작 위치
		region.imageExtent = {												// 이미지의 저장할 너비, 높이, 깊이
			width,
			height,
			1
		};

		// 커맨드 버퍼에 버퍼 -> 이미지로 데이터 복사하는 명령 기록
		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		// 커맨드 버퍼 기록 종료 및 제출
		endSingleTimeCommands(commandBuffer);
	}

	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
		// 이미지 포맷이 선형 필터링을 사용한 Blit 작업을 지원하는지 확인
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, imageFormat, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		// 커맨드 버퍼 생성 및 기록 시작
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		// 베리어 생성
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		// miplevel 0 ~ mipLevels
		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;								// 해당 mipmap에 대한 barrier 설정
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;					// 데이터 쓰기에 적합한 레이아웃
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;					// 데이터 읽기에 적합한 레이아웃
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;						// 쓰기 권한 on
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;						// 읽기 권한 on

			// 파이프라인 베리어 설정 (GPU 특정 작업간의 동기화 설정)
			// 이전 단계의 mipmap 복사가 끝나야, 다음 단계 mipmap 복사가 시작되게 베리어 설정
			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			// blit 작업시 소스와 대상의 복사 범위를 지정하는 구조체
			VkImageBlit blit{};
			blit.srcOffsets[0] = {0, 0, 0};
			blit.srcOffsets[1] = {mipWidth, mipHeight, 1};														// 소스의 범위 설정
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;																// 소스의 mipLevel 설정
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = {0, 0, 0};
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };		// 대상의 범위 설정
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;																	// 대상의 mipLevel 설정
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			// 소스 miplevel 을 대상 miplevel로 설정에 맞게 복사
			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR); // 선형적으로 복사

			// shader 단계에서 사용하기 전에 Bllit 단계가 끝나기를 기다림
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		// 마지막 단계 miplevel 처리
		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		endSingleTimeCommands(commandBuffer);
	}
};

class UniformBuffer : public Buffer {
public:
	static std::shared_ptr<UniformBuffer> createUniformBuffer(VkDeviceSize buffersize) {
		std::shared_ptr<UniformBuffer> uniformBuffer = std::shared_ptr<UniformBuffer>(new UniformBuffer());
		uniformBuffer->initUniformBuffer(buffersize);
		return uniformBuffer;
	}

	void cleanup() {
		vkDestroyBuffer(m_device, m_buffer, nullptr);
		vkFreeMemory(m_device, m_bufferMemory, nullptr);
	}

	~UniformBuffer() {}

	void updateUniformBuffer(void* data, VkDeviceSize size) {
		memcpy(m_mappedMemory, data, size);
	}

	//getter
	VkBuffer getBuffer() { return m_buffer; }
	VkDeviceMemory getBufferMemory() { return m_bufferMemory; }


private:
	void* m_mappedMemory = nullptr;

	void initUniformBuffer(VkDeviceSize buffersize) {
		auto &context = VulkanContext::getContext();
		m_device = context.getDevice();
		m_physicalDevice = context.getPhysicalDevice();
		m_commandPool = context.getCommandPool();
		m_graphicsQueue = context.getGraphicsQueue();

		createBuffer(buffersize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffer, m_bufferMemory);
		vkMapMemory(m_device, m_bufferMemory, 0, buffersize, 0, &m_mappedMemory);
	}
};


class Mesh {
public:
	static std::shared_ptr<Mesh> createMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
			std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(new Mesh());
			mesh->initMesh(vertices, indices);
			return mesh;
	}

	static std::shared_ptr<Mesh> createBox() {
		std::vector<Vertex> vertices = {
			Vertex { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 0.0f) },
			Vertex { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 0.0f) },
			Vertex { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 1.0f) },
			Vertex { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 1.0f) },

			Vertex { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 0.0f) },
			Vertex { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 0.0f) },
			Vertex { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 1.0f) },
			Vertex { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 1.0f) },

			Vertex { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
			Vertex { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
			Vertex { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
			Vertex { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 0.0f) },

			Vertex { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
			Vertex { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
			Vertex { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
			Vertex { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 0.0f) },

			Vertex { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
			Vertex { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
			Vertex { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
			Vertex { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(0.0f, 0.0f) },

			Vertex { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
			Vertex { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
			Vertex { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
			Vertex { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(0.0f, 0.0f) },
   		};

		std::vector<uint32_t> indices = {
			0,  2,  1,  2,  0,  3,
			4,  5,  6,  6,  7,  4,
			8,  9, 10, 10, 11,  8,
			12, 14, 13, 14, 12, 15,
			16, 17, 18, 18, 19, 16,
			20, 22, 21, 22, 20, 23,
		};

		return createMesh(vertices, indices);
	}

	static std::shared_ptr<Mesh> createSphere() {
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			uint32_t latiSegmentCount = 16;
			uint32_t longiSegmentCount = 32;

			uint32_t circleVertCount = longiSegmentCount + 1;
			vertices.resize((latiSegmentCount + 1) * circleVertCount);
			for (uint32_t i = 0; i <= latiSegmentCount; i++) {
				float v = (float)i / (float)latiSegmentCount;
				float phi = (v - 0.5f) * glm::pi<float>();
				auto cosPhi = cosf(phi);
				auto sinPhi = sinf(phi);
				for (uint32_t j = 0; j <= longiSegmentCount; j++) {
					float u = (float)j / (float)longiSegmentCount;
					float theta = u * glm::pi<float>() * 2.0f;
					auto cosTheta = cosf(theta);
					auto sinTheta = sinf(theta);
					auto point = glm::vec3(
						cosPhi * cosTheta, sinPhi, -cosPhi * sinTheta);
					
					vertices[i * circleVertCount + j] = Vertex {
						point * 0.5f, point, glm::vec2(u, v)
					};
				}
			}

			indices.resize(latiSegmentCount * longiSegmentCount * 6);
			for (uint32_t i = 0; i < latiSegmentCount; i++) {
				for (uint32_t j = 0; j < longiSegmentCount; j++) {
					uint32_t vertexOffset = i * circleVertCount + j;
					uint32_t indexOffset = (i * longiSegmentCount + j) * 6;
					indices[indexOffset    ] = vertexOffset;
					indices[indexOffset + 1] = vertexOffset + 1;
					indices[indexOffset + 2] = vertexOffset + 1 + circleVertCount;
					indices[indexOffset + 3] = vertexOffset;
					indices[indexOffset + 4] = vertexOffset + 1 + circleVertCount;
					indices[indexOffset + 5] = vertexOffset + circleVertCount;
				}
			}
		return createMesh(vertices, indices);
	}

	static std::shared_ptr<Mesh> createPlane() {
		std::vector<Vertex> vertices = {
			Vertex { glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
			Vertex { glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(1.0f, 0.0f) },
			Vertex { glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },
			Vertex { glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
		};

		std::vector<uint32_t> indices = {
			0,  1,  2,  2,  3,  0,
		};

		return createMesh(vertices, indices);
	}

	~Mesh() {}
	void draw(VkCommandBuffer commandBuffer) {	
		m_vertexBuffer->bind(commandBuffer);
		m_indexBuffer->bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, m_indexBuffer->getIndexCount(), 1, 0, 0, 0);
	}
	void cleanup() {
		m_vertexBuffer->cleanup();
		m_indexBuffer->cleanup();
	}
private:
	Mesh() {}

	std::unique_ptr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<IndexBuffer> m_indexBuffer;

	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkCommandPool m_commandPool;
	VkQueue m_graphicsQueue;


	void initMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
		auto& context = VulkanContext::getContext();
		m_device = context.getDevice();
		m_physicalDevice = context.getPhysicalDevice();
		m_commandPool = context.getCommandPool();
		m_graphicsQueue = context.getGraphicsQueue();

		m_vertexBuffer = VertexBuffer::createVertexBuffer(vertices);
		m_indexBuffer = IndexBuffer::createIndexBuffer(indices);
		
	}
};

class Model {
public:
	static std::shared_ptr<Model> createModel(std::string path) {
		std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
		model->initModel(path);
		return model;
	}

	static std::shared_ptr<Model> createBoxModel() {
		std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
		model->initBoxModel();
		return model;
	}

	static std::shared_ptr<Model> createSphereModel() {
		std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
		model->initSphereModel();
		return model;
	}

	static std::shared_ptr<Model> createPlaneModel() {
		std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
		model->initPlaneModel();
		return model;
	}

	~Model() {}
	void draw(VkCommandBuffer commandBuffer) {
		for (auto& mesh : m_meshes) {
			mesh->draw(commandBuffer);
		}
	}
	void cleanup() {
		for (auto& mesh : m_meshes) {
			mesh->cleanup();
		}
	}
private:
	Model() {}
	std::vector< std::shared_ptr<Mesh> > m_meshes;

	void initModel(std::string path) {

		loadModel(path);
	}

	void initBoxModel() {

		m_meshes.push_back(Mesh::createBox());
	}

	void initSphereModel() {
		m_meshes.push_back(Mesh::createSphere());
	}

	void initPlaneModel() {
		m_meshes.push_back(Mesh::createPlane());
	}


	void loadModel(std::string path) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			throw std::runtime_error("failed to load model!");
		}

		processNode(scene->mRootNode, scene);
	}

	void processNode(aiNode* node, const aiScene* scene) {
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_meshes.push_back(std::move(processMesh(mesh, scene)));
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
	}

	std::shared_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene) {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex{};
			vertex.pos = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
			vertex.color = {1.0f, 1.0f, 1.0f};
			if (mesh->mTextureCoords[0]) {
				vertex.texCoord = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
			} else {
				vertex.texCoord = {0.0f, 0.0f};
			}
			vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}

		return Mesh::createMesh(vertices, indices);
	}
};


class Texture : public Buffer {
public:
	static std::shared_ptr<Texture> createTexture(std::string path) {
		std::shared_ptr<Texture> texture = std::shared_ptr<Texture>(new Texture());
		texture->initTexture(path);
		return texture;
	}
	~Texture() {}
	void cleanup() {
		vkDestroySampler(device, textureSampler, nullptr);
		vkDestroyImageView(device, textureImageView, nullptr);
		m_imageBuffer->cleanup();
	}

	VkImageView getImageView() {
		return textureImageView;
	}

	VkSampler getSampler() {
		return textureSampler;
	}

private:
	Texture() {}

	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;

	uint32_t mipLevels;

	std::unique_ptr<ImageBuffer> m_imageBuffer;

	VkImageView textureImageView;
    VkSampler textureSampler;

	void initTexture(std::string path) {
		auto &context = VulkanContext::getContext();
		device = context.getDevice();
		physicalDevice = context.getPhysicalDevice();
		commandPool = context.getCommandPool();
		graphicsQueue = context.getGraphicsQueue();

		loadTexture(path);
	}

	void loadTexture(std::string path) {
		m_imageBuffer = ImageBuffer::createImageBuffer(path);
		mipLevels = m_imageBuffer->getMipLevels();
		createTextureImageView();
		createTextureSampler();
	}

	// 텍스처 이미지 뷰 생성
	void createTextureImageView() {
		// SRGB 총 4바이트 포맷으로 된 이미지 뷰 생성
		textureImageView = VulkanUtil::createImageView(m_imageBuffer->getImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_imageBuffer->getMipLevels());
	}

	// 텍스처를 위한 샘플러 생성 함수
	void createTextureSampler() {
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
};


class Object {
public:
	static std::unique_ptr<Object> createObject(std::shared_ptr<Model> model, std::shared_ptr<Texture> texture,
	glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
		std::unique_ptr<Object> object = std::unique_ptr<Object>(new Object());
		object->initObject(model, texture, position, rotation, scale);
		return object;
	}
	~Object() {}
	void draw(VkCommandBuffer commandBuffer) {
		m_model->draw(commandBuffer);
	}

	glm::mat4 getModelMatrix() {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, m_position);
		model = glm::rotate(model, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, m_scale);
		return model;
	}

	const std::shared_ptr<Texture>& getTexture() { return m_texture; }

private:
	Object() {}
	std::shared_ptr<Model> m_model;
	std::shared_ptr<Texture> m_texture;

	// material
	// 탄성
	// 마찰

	glm::vec3 m_position;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;

	void initObject(std::shared_ptr<Model> model, std::shared_ptr<Texture> texture,
	glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
		m_model = model;
		m_texture = texture;
		m_position = position;
		m_rotation = rotation;
		m_scale = scale;
	}
};

class Scene {
public:
	static std::unique_ptr<Scene> createScene() {
		std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new Scene());
		scene->initScene();
		return scene;
	}
	~Scene() {}

	void cleanup() { // object는 cleanup 해줄 필요 없지만 model과 texture는 해줘야함
		m_boxModel->cleanup();
		m_sphereModel->cleanup();
		m_planeModel->cleanup();

		m_vikingModel->cleanup();
		m_catModel->cleanup();

		m_vikingTexture->cleanup();
		m_sampleTexture->cleanup();
		m_catTexture->cleanup();
		m_karinaTexture->cleanup();
	}

	const std::vector< std::shared_ptr<Object> >& getObjects() { return m_objects; }

	glm::vec3 getLightPos() { return m_lightPos; }

	glm::vec3 getCamPos() {	return m_cameraPos; }

	glm::vec3 getCamFront() { return m_cameraFront; }

	glm::vec3 getCamUp() { return m_cameraUp; }

	float getCamPitch() { return m_cameraPitch; }

	float getCamYaw() { return m_cameraYaw; }

	size_t getObjectCount() { return m_objectCount; }

	glm::mat4 getViewMatrix() {
		return glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
	}

	glm::mat4 getProjMatrix(VkExtent2D swapChainExtent) {
		return glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 100.0f);
	}

private:
	Scene() {}
	std::shared_ptr<Model> m_boxModel;
	std::shared_ptr<Model> m_sphereModel;
	std::shared_ptr<Model> m_planeModel;

	// obj file
	std::shared_ptr<Model> m_vikingModel;
	std::shared_ptr<Model> m_catModel;


	std::shared_ptr<Texture> m_vikingTexture;
	std::shared_ptr<Texture> m_sampleTexture;
	std::shared_ptr<Texture> m_catTexture;
	std::shared_ptr<Texture> m_karinaTexture;

	std::vector< std::shared_ptr<Object> > m_objects;

	float m_cameraPitch { 0.0f };
    float m_cameraYaw { 0.0f };
    glm::vec3 m_cameraFront { glm::vec3(0.0f, 0.0f, -1.0f) };
    glm::vec3 m_cameraPos { glm::vec3(0.0f, 0.0f, 5.0f) };
    glm::vec3 m_cameraUp { glm::vec3(0.0f, 1.0f, 0.0f) };

    glm::vec3 m_lightPos { 0.0f, 10.0f, 0.0f };
	size_t m_objectCount;

	void initScene() {

		m_boxModel = Model::createBoxModel();
		m_sphereModel = Model::createSphereModel();
		m_planeModel = Model::createPlaneModel();

		m_vikingModel = Model::createModel("models/viking_room.obj");
		m_catModel = Model::createModel("models/cat.obj");

		//texture 해야함
		m_vikingTexture = Texture::createTexture("textures/viking_room.png");
		m_sampleTexture = Texture::createTexture("textures/texture.png");
		m_catTexture = Texture::createTexture("textures/cat.bmp");
		m_karinaTexture = Texture::createTexture("textures/karina.jpg");

		m_objects.push_back(Object::createObject(m_boxModel, m_sampleTexture, 
		glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));

		m_objects.push_back(Object::createObject(m_vikingModel, m_vikingTexture, 
		glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));

		m_objects.push_back(Object::createObject(m_sphereModel, m_vikingTexture, 
		glm::vec3(0.3f, 1.3f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));

		m_objects.push_back(Object::createObject(m_planeModel, m_karinaTexture, 
		glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0.0f, 0.0f, 180.0f), glm::vec3(5.0f * 0.74f, 5.0f, 1.0f)));
		
		m_objects.push_back(Object::createObject(m_catModel, m_catTexture, 
		glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(-90.0f, 0.0f, 45.0f), glm::vec3(0.01f, 0.01f, 0.01f)));


		m_objectCount = m_objects.size();
	}
};


class SwapChain {
public:
	static std::unique_ptr<SwapChain> createSwapChain(GLFWwindow* window) {
		std::unique_ptr<SwapChain> swapChain = std::unique_ptr<SwapChain>(new SwapChain());
		swapChain->initSwapChain(window);
		return swapChain;
	}

	void cleanup() {
		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(device, swapChain, nullptr);
	}

	void recreateSwapChain() {
		cleanup();
		createSwapChain();
		createImageViews();
	}

	VkSwapchainKHR getSwapChain() { return swapChain; }
	std::vector<VkImage>& getSwapChainImages() { return swapChainImages; }
	VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
	VkExtent2D getSwapChainExtent() { return swapChainExtent; }
	std::vector<VkImageView>& getSwapChainImageViews() { return swapChainImageViews; }





private:
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;


	GLFWwindow* window;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkSurfaceKHR surface;


	void initSwapChain(GLFWwindow* window) {
		this->window = window;

		auto& context = VulkanContext::getContext();
		device = context.getDevice();
		physicalDevice = context.getPhysicalDevice();
		surface = context.getSurface();

		createSwapChain();
		createImageViews();
	}

	void createSwapChain() {
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
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // 기본 렌더링에만 사용하는 플래그 (만약 렌더링 후 2차 가공 필요시 다른 플래그 사용)

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
	void createImageViews() {
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
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
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
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
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
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
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
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
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
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
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

};


class SyncObjects {
public:
	static std::unique_ptr<SyncObjects> createSyncObjects() {
		std::unique_ptr<SyncObjects> syncObjects = std::unique_ptr<SyncObjects>(new SyncObjects());
		syncObjects->initSyncObjects();
		return syncObjects;
	}

	~SyncObjects() {}

	void cleanup() {
		auto& context = VulkanContext::getContext();
		VkDevice device = context.getDevice();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}
	}

	std::vector<VkSemaphore>& getImageAvailableSemaphores() { return imageAvailableSemaphores; }
	std::vector<VkSemaphore>& getRenderFinishedSemaphores() { return renderFinishedSemaphores; }
	std::vector<VkFence>& getInFlightFences() { return inFlightFences; }

private:
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	void initSyncObjects() {
		auto& context = VulkanContext::getContext();
		VkDevice device = context.getDevice();

		// 세마포어, 펜스 vector 동시에 처리할 최대 프레임 버퍼 수만큼 할당
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		// 세마포어 생성 설정 값 준비
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		// 펜스 생성 설정 값 준비
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;         // signal 등록된 상태로 생성 (시작하자마자 wait으로 시작하므로 필요한 FLAG)

		// 세마포어, 펜스 생성
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}
};


class CommandBuffers {
public:
	static std::unique_ptr<CommandBuffers> createCommandBuffers() {
		std::unique_ptr<CommandBuffers> commandBuffers = std::unique_ptr<CommandBuffers>(new CommandBuffers());
		commandBuffers->initCommandBuffers();
		return commandBuffers;
	}

	~CommandBuffers() {}

	void cleanup() {
		auto& context = VulkanContext::getContext();
		VkDevice device = context.getDevice();

		vkFreeCommandBuffers(device, context.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	}

	std::vector<VkCommandBuffer>& getCommandBuffers() { return commandBuffers; }

private:
	std::vector<VkCommandBuffer> commandBuffers;

	/*
	[커맨드 버퍼 생성]
	커맨드 버퍼에 GPU에서 실행할 작업을 전부 기록한뒤 제출한다.
	GPU는 해당 커맨드 버퍼의 작업을 알아서 실행하고, CPU는 다른 일을 할 수 있게 된다. (병렬 처리)
	*/
	void initCommandBuffers() {
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

};


class RenderPass {
public:
	static std::unique_ptr<RenderPass> createRenderPass(VkFormat swapChainImageFormat) {
		std::unique_ptr<RenderPass> renderPass = std::unique_ptr<RenderPass>(new RenderPass());
		renderPass->initRenderPass(swapChainImageFormat);
		return renderPass;
	}

	void cleanup(){
		auto& context = VulkanContext::getContext();
		VkDevice device = context.getDevice();

		vkDestroyRenderPass(device, renderPass, nullptr);
	}

	VkRenderPass getRenderPass() { return renderPass; }

private:
	VkRenderPass renderPass;

	void initRenderPass(VkFormat swapChainImageFormat) {
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


};


class FrameBuffers {
public:
	static std::unique_ptr<FrameBuffers> createSwapChainFrameBuffers(SwapChain* swapChain, VkRenderPass renderPass) {
		std::unique_ptr<FrameBuffers> frameBuffers = std::unique_ptr<FrameBuffers>(new FrameBuffers());
		frameBuffers->initSwapChainFrameBuffers(swapChain, renderPass);
		return frameBuffers;
	}
	~FrameBuffers() {}

	void cleanup() {
		auto& context = VulkanContext::getContext();
		VkDevice device = context.getDevice();

		vkDestroyImageView(device, colorImageView, nullptr);
		vkDestroyImage(device, colorImage, nullptr);
		vkFreeMemory(device, colorImageMemory, nullptr);

		vkDestroyImageView(device, depthImageView, nullptr);
		vkDestroyImage(device, depthImage, nullptr);
		vkFreeMemory(device, depthImageMemory, nullptr);

		for (auto framebuffer : framebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
	}

	void initSwapChainFrameBuffers(SwapChain* swapChain, VkRenderPass renderPass) {
		
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

		framebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			std::array<VkImageView, 3> attachments = {
				colorImageView,
				depthImageView,
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

	std::vector<VkFramebuffer>& getFramebuffers() { return framebuffers; }

private:
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	std::vector<VkFramebuffer> framebuffers;
};


class DescriptorSetLayout {
public:
	static std::unique_ptr<DescriptorSetLayout> createDescriptorSetLayout() {
		std::unique_ptr<DescriptorSetLayout> descriptorSetLayout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
		descriptorSetLayout->initDescriptorSetLayout();
		return descriptorSetLayout;
	}

	~DescriptorSetLayout() {}

	void cleanup() {
		auto& context = VulkanContext::getContext();
		VkDevice device = context.getDevice();

		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	}

	VkDescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; }

private:
	VkDescriptorSetLayout descriptorSetLayout;

	void initDescriptorSetLayout() {
		auto& context = VulkanContext::getContext();
		VkDevice device = context.getDevice();

		// 디스크립터 레이아웃 설정
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;										// 바인딩 포인트 설정
		uboLayoutBinding.descriptorCount = 1;								// 디스크립터 개수 설정
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;	// 디스크립터 타입 설정
		uboLayoutBinding.pImmutableSamplers = nullptr;						// 이미지 샘플러 설정
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;			// 셰이더 타입 설정

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}
};


class ShaderResourceManager {
public:
	static std::unique_ptr<ShaderResourceManager> createShaderResourceManager(Scene* scene, VkDescriptorSetLayout descriptorSetLayout) {
		std::unique_ptr<ShaderResourceManager> shaderResourceManager = std::unique_ptr<ShaderResourceManager>(new ShaderResourceManager());
		shaderResourceManager->initShaderResourceManager(scene, descriptorSetLayout);
		return shaderResourceManager;
	}

	~ShaderResourceManager() {}

	void cleanup() {
		for (size_t i = 0; i < m_uniformBuffers.size(); i++) {
			m_uniformBuffers[i]->cleanup();
		}
	}

	std::vector<std::shared_ptr<UniformBuffer>>& getUniformBuffers() { return m_uniformBuffers; }
	std::vector<VkDescriptorSet>& getDescriptorSets() { return descriptorSets; }


private:
	std::vector<std::shared_ptr<UniformBuffer>> m_uniformBuffers;
	std::vector<VkDescriptorSet> descriptorSets;

	void initShaderResourceManager(Scene* scene, VkDescriptorSetLayout descriptorSetLayout) {
		createUniformBuffers(scene);
		createDescriptorSets(scene, descriptorSetLayout);
	}

	void createUniformBuffers(Scene* scene) {
		size_t objectCount = scene->getObjectCount();
		if (objectCount == 0) {
			throw std::runtime_error("failed to create uniform buffers!");
		}
		// 유니폼 버퍼에 저장 될 구조체의 크기
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		// 각 요소들을 동시에 처리 가능한 최대 프레임 수만큼 만들어 둔다.
		m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT * objectCount);

		for (size_t i = 0; i < objectCount; i++) {
			for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
				m_uniformBuffers[i * MAX_FRAMES_IN_FLIGHT + j] = UniformBuffer::createUniformBuffer(bufferSize);
			}
		}
	}

	void createDescriptorSets(Scene* scene, VkDescriptorSetLayout descriptorSetLayout) {
		auto& context = VulkanContext::getContext();
		VkDevice device = context.getDevice();
		VkDescriptorPool descriptorPool = context.getDescriptorPool();
	
		size_t objectCount = scene->getObjectCount();
		std::vector<std::shared_ptr<Object>> objects = scene->getObjects();

		if (objectCount == 0) {
			throw std::runtime_error("failed to create descriptor sets!");
		}
		// 디스크립터 셋 레이아웃 벡터 생성 (기존 만들어놨던 디스크립터 셋 레이아웃 객체 이용)
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT * objectCount, descriptorSetLayout);

		// 디스크립터 셋 할당에 필요한 정보를 설정하는 구조체
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;										// 디스크립터 셋을 할당할 디스크립터 풀 지정
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * objectCount);		// 할당할 디스크립터 셋 개수 지정
		allocInfo.pSetLayouts = layouts.data();											// 할당할 디스크립터 셋 의 레이아웃을 정의하는 배열 

		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT * objectCount);									// 디스크립터 셋을 저장할 벡터 크기 설정
		
		// 디스크립터 풀에 디스크립터 셋 할당
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}


		for (size_t i = 0; i < objectCount; i++) {
			for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
				// 디스크립터 셋에 바인딩할 버퍼 정보 
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = m_uniformBuffers[i * MAX_FRAMES_IN_FLIGHT + j]->getBuffer();			// 바인딩할 버퍼
				bufferInfo.offset = 0;												// 버퍼에서 데이터 시작 위치 offset
				bufferInfo.range = sizeof(UniformBufferObject);						// 셰이더가 접근할 버퍼 크기

				std::shared_ptr<Texture> texture = objects[i]->getTexture();
				VkDescriptorImageInfo imageInfo{};								
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// 이미지의 레이아웃
				imageInfo.imageView = texture->getImageView();						// 셰이더에서 사용할 이미지 뷰
				imageInfo.sampler = texture->getSampler();							// 이미지 샘플링에 사용할 샘플러 설정
				
				// 디스크립터 셋 바인딩 및 업데이트
				std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = descriptorSets[i * MAX_FRAMES_IN_FLIGHT + j];					// 업데이트 할 디스크립터 셋
				descriptorWrites[0].dstBinding = 0;													// 업데이트 할 바인딩 포인트
				descriptorWrites[0].dstArrayElement = 0;											// 업데이트 할 디스크립터가 배열 타입인 경우 해당 배열의 원하는 index 부터 업데이트 가능 (배열 아니면 0으로 지정)
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;				// 업데이트 할 디스크립터 타입
				descriptorWrites[0].descriptorCount = 1;											// 업데이트 할 디스크립터 개수
				descriptorWrites[0].pBufferInfo = &bufferInfo;										// 업데이트 할 버퍼 디스크립터 정보 구조체 배열

				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = descriptorSets[i * MAX_FRAMES_IN_FLIGHT + j];					// 업데이트 할 디스크립터 셋
				descriptorWrites[1].dstBinding = 1;													// 업데이트 할 바인딩 포인트
				descriptorWrites[1].dstArrayElement = 0;											// 업데이트 할 디스크립터가 배열 타입인 경우 해당 배열의 원하는 index 부터 업데이트 가능 (배열 아니면 0으로 지정)
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;		// 업데이트 할 디스크립터 타입
				descriptorWrites[1].descriptorCount = 1;											// 업데이트 할 디스크립터 개수
				descriptorWrites[1].pImageInfo = &imageInfo;										// 업데이트 할 버퍼 디스크립터 정보 구조체 배열

				// 디스크립터 셋을 업데이트 하여 사용할 리소스 바인딩
				vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
		}
	}
};

class Pipeline {
public:
	static std::unique_ptr<Pipeline> createPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout) {
		std::unique_ptr<Pipeline> pipeline = std::unique_ptr<Pipeline>(new Pipeline());
		pipeline->initPipeline(renderPass, descriptorSetLayout);
		return pipeline;
	}

	~Pipeline() {}

	void cleanup() {
		auto& context = VulkanContext::getContext();
		VkDevice device = context.getDevice();

		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	}

	VkPipeline getPipeline() { return pipeline; }
	VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

private:

	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	void initPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout) {
		auto& context = VulkanContext::getContext();
		VkDevice device = context.getDevice();
		VkSampleCountFlagBits msaaSamples = context.getMsaaSamples();

		// SPIR-V 파일 읽기
		std::vector<char> vertShaderCode = VulkanUtil::readFile("./shaders/vert.spv");
		std::vector<char> fragShaderCode = VulkanUtil::readFile("./shaders/frag.spv");

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
	VkShaderModule createShaderModule(const std::vector<char>& code) {
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
};

class Renderer {
public:
	static std::unique_ptr<Renderer> createRenderer(GLFWwindow* window) {
		std::unique_ptr<Renderer> renderer = std::unique_ptr<Renderer>(new Renderer());
		renderer->init(window);
		return renderer;
	}
	~Renderer() {}

	void loadScene(Scene* scene) {
		m_shaderResourceManager = ShaderResourceManager::createShaderResourceManager(scene, m_descriptorSetLayout->getDescriptorSetLayout());
		descriptorSets = m_shaderResourceManager->getDescriptorSets();
		m_uniformBuffers = m_shaderResourceManager->getUniformBuffers();


	}

	// getter
	VkDevice getDevice() {
		return device;
	}

	void drawFrame(Scene* scene) {
		// [이전 GPU 작업 대기]
		// 동시에 작업 가능한 최대 Frame 개수만큼 작업 중인 경우 대기 (가장 먼저 시작한 Frame 작업이 끝나서 Fence에 signal을 보내기를 기다림)
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
 
		// [작업할 image 준비]
		// 이번 Frame 에서 사용할 이미지 준비 및 해당 이미지 index 받아오기 (준비가 끝나면 signal 보낼 세마포어 등록)
		// vkAcquireNextImageKHR 함수는 CPU에서 swapChain과 surface의 호환성을 확인하고 GPU에 이미지 준비 명령을 내리는 함수
		// 만약 image가 프레젠테이션 큐에 작업이 진행 중이거나 대기 중이면 해당 image는 사용하지 않고 대기한다.
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		// image 준비 실패로 인한 오류 처리
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			// 스왑 체인이 surface 크기와 호환되지 않는 경우로(창 크기 변경), 스왑 체인 재생성 후 다시 draw
			recreateSwapChain();
			return;
		} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			// 진짜 오류 gg
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		// [Fence 초기화]
		// Fence signal 상태 not signaled 로 초기화
		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		// [Command Buffer에 명령 기록]
		// 커맨드 버퍼 초기화 및 명령 기록
		vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0); // 두 번째 매개변수인 Flag 를 0으로 초기화하면 기본 초기화 진행
		recordCommandBuffer(scene, commandBuffers[currentFrame], imageIndex); // 현재 작업할 image의 index와 commandBuffer를 전송

		// [렌더링 Command Buffer 제출]
		// 렌더링 커맨드 버퍼 제출 정보 객체 생성
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// 작업 실행 신호를 받을 대기 세마포어 설정 (해당 세마포어가 signal 상태가 되기 전엔 대기)
		VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};				
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; 	
		submitInfo.waitSemaphoreCount = 1;														// 대기 세마포어 개수
		submitInfo.pWaitSemaphores = waitSemaphores;											// 대기 세마포어 등록
		submitInfo.pWaitDstStageMask = waitStages;												// 대기할 시점 등록 (그 전까지는 세마포어 상관없이 그냥 진행)	

		// 커맨드 버퍼 등록
		submitInfo.commandBufferCount = 1;														// 커맨드 버퍼 개수 등록
		submitInfo.pCommandBuffers = &commandBuffers[currentFrame];								// 커매드 버퍼 등록

		// 작업이 완료된 후 신호를 보낼 세마포어 설정 (작업이 끝나면 해당 세마포어 signal 상태로 변경)
		VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
		submitInfo.signalSemaphoreCount = 1;													// 작업 끝나고 신호를 보낼 세마포어 개수
		submitInfo.pSignalSemaphores = signalSemaphores;										// 작업 끝나고 신호를 보낼 세마포어 등록

		// 커맨드 버퍼 제출 
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		// [프레젠테이션 Command Buffer 제출]
		// 프레젠테이션 커맨드 버퍼 제출 정보 객체 생성
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		// 작업 실행 신호를 받을 대기 세마포어 설정
		presentInfo.waitSemaphoreCount = 1;														// 대기 세마포어 개수
		presentInfo.pWaitSemaphores = signalSemaphores;											// 대기 세마포어 등록

		// 제출할 스왑 체인 설정
		VkSwapchainKHR swapChains[] = {swapChain};
		presentInfo.swapchainCount = 1;															// 스왑체인 개수
		presentInfo.pSwapchains = swapChains;													// 스왑체인 등록
		presentInfo.pImageIndices = &imageIndex;												// 스왑체인에서 표시할 이미지 핸들 등록

		// 프레젠테이션 큐에 이미지 제출
		result = vkQueuePresentKHR(presentQueue, &presentInfo);

		// 프레젠테이션 실패 오류 발생 시
		// if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) { <- framebufferResized는 명시적으로 해줄뿐 사실상 필요하지가 않음 나중에 수정할꺼면 하자
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			// 스왑 체인 크기와 surface의 크기가 호환되지 않는 경우
			recreateSwapChain(); 	// 변경된 surface에 맞는 SwapChain, ImageView, FrameBuffer 생성 
		} else if (result != VK_SUCCESS) {
			// 진짜 오류 gg
			throw std::runtime_error("failed to present swap chain image!");
		}
		// [프레임 인덱스 증가]
		// 다음 작업할 프레임 변경
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void cleanup() {
		// 스왑 체인 파괴

		m_swapChainFrameBuffers->cleanup();
		m_swapChain->cleanup();

		// vkDestroyPipeline(device, graphicsPipeline, nullptr);      	// 파이프라인 객체 삭제
		// vkDestroyPipelineLayout(device, pipelineLayout, nullptr);  	// 파이프라인 레이아웃 삭제
		m_pipeline->cleanup();
		
		m_renderPass->cleanup();
		// vkDestroyRenderPass(device, renderPass, nullptr);         	// 렌더 패스 삭제


		// for (size_t i = 0; i < m_uniformBuffers.size(); i++) {
		// 	m_uniformBuffers[i]->cleanup();
		// }
		m_shaderResourceManager->cleanup();


		m_descriptorSetLayout->cleanup();
		// vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);	// 디스크립터 셋 레이아웃 삭제

		m_syncObjects->cleanup();

		VulkanContext::getContext().cleanup();
	}

	/*
		변경된 window 크기에 맞게 SwapChain, ImageView, FrameBuffer 재생성
	*/
	void recreateSwapChain() {
		// 현재 프레임버퍼 사이즈 체크
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		
		// 현재 프레임 버퍼 사이즈가 0이면 다음 이벤트 호출까지 대기
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents(); // 다음 이벤트 발생 전까지 대기하여 CPU 사용률을 줄이는 함수 
		}

		// 모든 GPU 작업 종료될 때까지 대기 (사용중인 리소스를 건들지 않기 위해)
		vkDeviceWaitIdle(device);

		// 스왑 체인 관련 리소스 정리

		m_swapChainFrameBuffers->cleanup();
		m_swapChain->recreateSwapChain();

		// 현재 window 크기에 맞게 SwapChain, DepthResource, ImageView, FrameBuffer 재생성
		// createSwapChain();
		// createImageViews();
		swapChain = m_swapChain->getSwapChain();
		swapChainImages = m_swapChain->getSwapChainImages();
		swapChainImageFormat = m_swapChain->getSwapChainImageFormat();
		swapChainExtent = m_swapChain->getSwapChainExtent();
		swapChainImageViews = m_swapChain->getSwapChainImageViews();

		// createColorResources();
		// createDepthResources();
		// createFramebuffers();
		m_swapChainFrameBuffers->initSwapChainFrameBuffers(m_swapChain.get(), renderPass);
		swapChainFramebuffers = m_swapChainFrameBuffers->getFramebuffers();
	}

	

private:
	GLFWwindow* window;
	VkSurfaceKHR surface;
	// device
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkDevice device;
	
	// command
	VkCommandPool commandPool;
	VkQueue graphicsQueue;
	VkQueue presentQueue;


	std::unique_ptr<SwapChain> m_swapChain;

	// swapchain
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;

	std::unique_ptr<FrameBuffers> m_swapChainFrameBuffers;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	// renderpass
	std::unique_ptr<RenderPass> m_renderPass;

	VkRenderPass renderPass;
	
	std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
	VkDescriptorSetLayout descriptorSetLayout;


	std::unique_ptr<Pipeline> m_pipeline;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	std::vector< std::shared_ptr<UniformBuffer> > m_uniformBuffers;

	VkDescriptorPool descriptorPool;

	std::vector<VkDescriptorSet> descriptorSets;
	std::unique_ptr<ShaderResourceManager> m_shaderResourceManager;
	
	std::unique_ptr<CommandBuffers> m_commandBuffers;
	std::vector<VkCommandBuffer> commandBuffers;

	std::unique_ptr<SyncObjects> m_syncObjects;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	uint32_t currentFrame = 0;


	Renderer() {}
	void init(GLFWwindow* window) {


		this->window = window;

		auto &context = VulkanContext::getContext();
		context.initContext(window);
		surface = context.getSurface();
		physicalDevice = context.getPhysicalDevice();
		device = context.getDevice();
		graphicsQueue = context.getGraphicsQueue();
		presentQueue = context.getPresentQueue();
		commandPool = context.getCommandPool();
		msaaSamples = context.getMsaaSamples();
		descriptorPool = context.getDescriptorPool();

		m_swapChain = SwapChain::createSwapChain(window);
		swapChain = m_swapChain->getSwapChain();
		swapChainImages = m_swapChain->getSwapChainImages();
		swapChainImageFormat = m_swapChain->getSwapChainImageFormat();
		swapChainExtent = m_swapChain->getSwapChainExtent();
		swapChainImageViews = m_swapChain->getSwapChainImageViews();

		m_syncObjects = SyncObjects::createSyncObjects();
		imageAvailableSemaphores = m_syncObjects->getImageAvailableSemaphores();
		renderFinishedSemaphores = m_syncObjects->getRenderFinishedSemaphores();
		inFlightFences = m_syncObjects->getInFlightFences();

		// createInstance();
		// setupDebugMessenger();
		// createSurface();

		// pickPhysicalDevice();
		// createLogicalDevice();


		// createSwapChain();
		// createImageViews();
			// createSyncObjects();

		m_renderPass = RenderPass::createRenderPass(swapChainImageFormat);
		renderPass = m_renderPass->getRenderPass();

		// createRenderPass();
		m_descriptorSetLayout = DescriptorSetLayout::createDescriptorSetLayout();
		descriptorSetLayout = m_descriptorSetLayout->getDescriptorSetLayout();
			// createDescriptorSetLayout();
			// createGraphicsPipeline();
		m_pipeline = Pipeline::createPipeline(renderPass, descriptorSetLayout);
		pipelineLayout = m_pipeline->getPipelineLayout();
		graphicsPipeline = m_pipeline->getPipeline();

		// createCommandPool();
		// createCommandBuffers();
		m_commandBuffers = CommandBuffers::createCommandBuffers();
		commandBuffers = m_commandBuffers->getCommandBuffers();


		m_swapChainFrameBuffers = FrameBuffers::createSwapChainFrameBuffers(m_swapChain.get(), renderPass);
		swapChainFramebuffers = m_swapChainFrameBuffers->getFramebuffers();

		// FrameBuffer class 로 묶음
		// createColorResources();
		// createDepthResources();
		// createFramebuffers();
		// 여기까지

			// loadModel();
			// createVertexBuffer();
			// createIndexBuffer();
			// createUniformBuffers();

			// createTextureImage();
			// createTextureImageView();
			// createTextureSampler();

		// createDescriptorPool();
		// createDescriptorSets();
	}





	/*
		[커맨드 버퍼에 작업 기록]
		1. 커맨드 버퍼 기록 시작
		2. 렌더패스 시작하는 명령 기록
		3. 파이프라인 설정 명령 기록
		4. 렌더링 명령 기록
		5. 렌더 패스 종료 명령 기록
		6. 커맨드 버퍼 기록 종료
	*/
	void recordCommandBuffer(Scene* scene, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		
		// 커맨드 버퍼 기록을 위한 정보 객체
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		// GPU에 필요한 작업을 모두 커맨드 버퍼에 기록하기 시작
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		// 렌더 패스 정보 지정
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;								// 렌더 패스 등록
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];		// 프레임 버퍼 등록
		renderPassInfo.renderArea.offset = {0, 0};							// 렌더링 시작 좌표 등록
		renderPassInfo.renderArea.extent = swapChainExtent;					// 렌더링 width, height 등록 (보통 프레임버퍼, 스왑체인의 크기와 같게 설정)

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};				

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());		// clear color 개수 등록
		renderPassInfo.pClearValues = clearValues.data();								// clear color 등록 (첨부한 attachment 개수와 같게 등록)
		
		/* 
			[렌더 패스를 시작하는 명령을 기록] 
			GPU에서 렌더링에 필요한 자원과 설정을 준비 (대략 과정)
			1. 렌더링 자원 초기화 (프레임 버퍼와 렌더 패스에 등록된 attachment layout 초기화)
			2. 서브패스 및 attachment 설정 적용
			3. 렌더링 작업을 위한 컨텍스트 준비 (뷰포트, 시저 등 설정)
		*/
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		//	[사용할 그래픽 파이프 라인을 설정하는 명령 기록]
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		// 뷰포트 정보 입력
		VkViewport viewport{};
		viewport.x = 0.0f;									// 뷰포트의 시작 x 좌표
		viewport.y = 0.0f;									// 뷰포트의 시작 y 좌표
		viewport.width = (float) swapChainExtent.width;		// 뷰포트의 width 크기
		viewport.height = (float) swapChainExtent.height;	// 뷰포트의 height 크기
		viewport.minDepth = 0.0f;							// 뷰포트의 최소 깊이
		viewport.maxDepth = 1.0f;							// 뷰포트의 최대 깊이
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);	// [커맨드 버퍼에 뷰포트 설정 등록]

		// 시저 정보 입력
		VkRect2D scissor{};
		scissor.offset = {0, 0};							// 시저의 시작 좌표
		scissor.extent = swapChainExtent;					// 시저의 width, height
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);		// [커맨드 버퍼에 시저 설정 등록]
		

		// [렌더링 명령 기록]
		const std::vector<std::shared_ptr<Object>>& objects = scene->getObjects();
		size_t objectCount = scene->getObjectCount();

		for (size_t i = 0; i < objectCount; i++) {
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[MAX_FRAMES_IN_FLIGHT * i + currentFrame], 0, nullptr);
			UniformBufferObject ubo{};
			ubo.model = objects[i]->getModelMatrix();
			ubo.view = scene->getViewMatrix();
			ubo.proj = scene->getProjMatrix(swapChainExtent);
			ubo.proj[1][1] *= -1;
			// memcpy(uniformBuffersMapped[currentFrame * objectCount + i], &ubo, sizeof(ubo));
			m_uniformBuffers[MAX_FRAMES_IN_FLIGHT * i + currentFrame]->updateUniformBuffer(&ubo, sizeof(ubo));
			objects[i]->draw(commandBuffer);
		}
		/*
			[렌더 패스 종료]
			1. 자원의 정리 및 레이아웃 전환 (최종 작업을 위해 attachment에 정의된 finalLayout 설정)
			2. Load, Store 작업 (각 attachment에 정해진 load, store 작업 실행)
			3. 렌더 패스의 종료를 GPU에 알려 자원 재활용 등이 가능해짐
		*/ 
		vkCmdEndRenderPass(commandBuffer);

		// [커맨드 버퍼 기록 종료]
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
};


class App {
public:
	void run() {
		window = Window::createWindow();
		renderer = Renderer::createRenderer(window->getWindow());
		scene = Scene::createScene();
		
		// renderer->createUniformBuffers(scene.get());
		// renderer->createDescriptorSets(scene.get());
		renderer->loadScene(scene.get());

		mainLoop();
		cleanup();
	}

private:
	std::unique_ptr<Window> window;
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<Scene> scene;
	// 물리

	/*
		렌더링 루프 실행	
	*/
	void mainLoop() {
		while (!glfwWindowShouldClose(window->getWindow())) {
			glfwPollEvents();
			renderer->drawFrame(scene.get());
		}
		vkDeviceWaitIdle(renderer->getDevice());  // 종료시 실행 중인 GPU 작업을 전부 기다림
	}

	void cleanup() {
		scene->cleanup();
		renderer->cleanup();
		window->cleanup();
	}
};

int main() {
	App app;

	try {
		app.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}