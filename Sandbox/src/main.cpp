#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// 限制同时进行的帧数，用于帧同步
const int MAX_FRAMES_IN_FLIGHT = 2;

// 验证层用于调试和验证Vulkan调用的正确性
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

// 设备扩展，VK_KHR_SWAPCHAIN_EXTENSION_NAME是交换链必需的扩展
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// 根据是否为调试模式决定是否启用验证层
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// 创建调试工具信使的辅助函数
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// 销毁调试工具信使的辅助函数
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

// 队列族索引结构体，用于存储支持图形命令和呈现命令的队列族索引
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;  // 支持图形操作的队列族索引
    std::optional<uint32_t> presentFamily;   // 支持呈现操作的队列族索引

    // 检查是否找到了支持所需操作的队列族
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

// 交换链支持详情结构体，存储物理设备对交换链的支持情况
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;     // 表面基础属性
    std::vector<VkSurfaceFormatKHR> formats;   // 支持的表面格式列表
    std::vector<VkPresentModeKHR> presentModes; // 支持的呈现模式列表
};

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;                      // GLFW窗口句柄

    VkInstance instance;                     // Vulkan实例，与驱动程序连接
    VkDebugUtilsMessengerEXT debugMessenger; // 调试信使对象
    VkSurfaceKHR surface;                    // 窗口表面，用于与窗口系统交互

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;  // 物理设备（GPU）
    VkDevice device;                           // 逻辑设备，与物理设备交互的接口

    VkQueue graphicsQueue;                   // 图形队列，用于执行图形命令
    VkQueue presentQueue;                    // 呈现队列，用于显示图像到表面

    VkSwapchainKHR swapChain;                // 交换链，管理一系列图像用于呈现
    std::vector<VkImage> swapChainImages;    // 交换链中的图像
    VkFormat swapChainImageFormat;           // 交换链图像格式
    VkExtent2D swapChainExtent;              // 交换链图像范围（分辨率）
    std::vector<VkImageView> swapChainImageViews;      // 图像视图，指定如何访问图像数据
    std::vector<VkFramebuffer> swapChainFramebuffers;  // 帧缓冲区，指定渲染目标

    VkRenderPass renderPass;                 // 渲染过程，描述渲染操作的结构和依赖
    VkPipelineLayout pipelineLayout;         // 管线布局，定义着色器可访问的资源
    VkPipeline graphicsPipeline;             // 图形管线，定义顶点处理和片段着色方式

    VkCommandPool commandPool;               // 命令池，管理命令缓冲区的内存
    VkCommandBuffer commandBuffer;           // 命令缓冲区，记录命令供执行

    VkSemaphore imageAvailableSemaphore;     // 信号量，表示图像已可用
    VkSemaphore renderFinishedSemaphore;     // 信号量，表示渲染已完成
    VkFence inFlightFence;                   // 围栏，用于CPU-GPU同步

    void initWindow() {
        glfwInit();

        // 配置GLFW不创建OpenGL上下文，并且窗口大小不可变
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan() {
        createInstance();           // 创建Vulkan实例
        setupDebugMessenger();     // 设置调试回调
        createSurface();           // 创建窗口表面
        pickPhysicalDevice();      // 选择合适的物理设备（GPU）
        createLogicalDevice();     // 创建逻辑设备
        createSwapChain();         // 创建交换链
        createImageViews();        // 创建图像视图
        createRenderPass();        // 创建渲染过程
        createGraphicsPipeline();  // 创建图形管线
        createFramebuffers();      // 创建帧缓冲区
        createCommandPool();       // 创建命令池
        createCommandBuffer();     // 创建命令缓冲区
        createSyncObjects();       // 创建同步对象
    }

    void mainLoop() {
        // 主循环，直到窗口被关闭
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();  // 处理窗口事件
            drawFrame();       // 绘制一帧
        }

        // 等待设备空闲，确保所有操作完成
        vkDeviceWaitIdle(device);
    }

    void cleanup() {
        // 清理所有创建的Vulkan对象（以创建的相反顺序销毁）
        vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
        vkDestroyFence(device, inFlightFence, nullptr);

        vkDestroyCommandPool(device, commandPool, nullptr);

        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void createInstance() {
        // 如果启用了验证层但不支持，则抛出异常
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        // 应用程序信息结构体，指定应用名称、版本等
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        // 实例创建信息结构体
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // 获取GLFW所需的扩展
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // 调试信使创建信息结构体
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            // 启用验证层
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            // 填充调试信使创建信息
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            // 不启用验证层
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        // 创建Vulkan实例
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    // 填充调试信使创建信息结构体
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        // 指定接收的消息级别
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        // 指定接收的消息类型
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        // 指定回调函数
        createInfo.pfnUserCallback = debugCallback;
    }

    // 设置调试信使
    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        // 创建调试信使
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    // 创建窗口表面
    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    // 选择合适的物理设备（GPU）
    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        // 获取物理设备数量
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        // 如果没有找到支持Vulkan的设备，抛出异常
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        // 获取所有物理设备
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // 遍历设备，找到第一个合适的设备
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        // 如果没有找到合适的设备，抛出异常
        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    // 创建逻辑设备
    void createLogicalDevice() {
        // 查找合适的队列族
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        // 创建队列创建信息列表
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        // 为每个唯一的队列族创建队列创建信息
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // 指定需要的设备特性（这里暂时不需要特殊特性）
        VkPhysicalDeviceFeatures deviceFeatures{};

        // 逻辑设备创建信息
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // 创建逻辑设备
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        // 获取图形队列和呈现队列的句柄
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    // 创建交换链
    void createSwapChain() {
        // 查询交换链支持详情
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        // 选择最佳的表面格式、呈现模式和交换链范围
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        // 确定交换链图像数量
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        // 交换链创建信息
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        
        // 指定交换链图像的数组层数（除非开发立体3D应用，否则总是1）
        createInfo.imageArrayLayers = 1;
        // 指定交换链图像的用途（直接渲染到这些图像上）
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        // 获取队列族索引
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        // 如果图形队列族和呈现队列族不同，则使用并发共享模式
        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            // 否则使用独占模式
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        // 指定变换（这里使用默认变换）
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        // 指定alpha通道（这里设置为不透明）
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        // 使用之前选择的呈现模式
        createInfo.presentMode = presentMode;
        // 允许裁剪（隐藏的像素不需要更新）
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        // 创建交换链
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        // 获取交换链图像
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        // 保存交换链图像格式和范围
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    // 创建图像视图
    void createImageViews() {
        // 调整图像视图向量大小以匹配交换链图像数量
        swapChainImageViews.resize(swapChainImages.size());

        // 为每个交换链图像创建图像视图
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            
            // 指定图像类型（2D纹理）
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            // 使用交换链图像格式
            createInfo.format = swapChainImageFormat;
            
            // 指定颜色通道映射（默认的RGBA映射）
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            
            // 指定图像子资源范围
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            // 创建图像视图
            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    void createRenderPass() {
        // 颜色附件描述
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;          // 使用交换链图像格式
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;        // 不使用多重采样
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // 渲染前清除帧缓冲区
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // 渲染后存储结果到内存
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;  // 不关心模板缓冲区
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;        // 初始布局未定义
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;    // 最终布局为呈现

        // 颜色附件引用
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;  // 引用附件数组中的第0个附件
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // 使用时的最佳布局

        // 子过程描述（渲染过程的核心）
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // 这是一个图形管线
        subpass.colorAttachmentCount = 1;                            // 一个颜色附件
        subpass.pColorAttachments = &colorAttachmentRef;             // 指向颜色附件引用

        // 子过程依赖关系（控制执行顺序和内存访问）
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;              // 外部子过程
        dependency.dstSubpass = 0;                                // 我们的子过程
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;  // 源阶段
        dependency.srcAccessMask = 0;                             // 源访问
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;  // 目标阶段
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;          // 目标访问

        // 渲染过程创建信息
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        // 创建渲染过程
        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void createGraphicsPipeline() {
        // 读取预编译的着色器代码
        auto vertShaderCode = readFile("shaders/vert.spv");
        auto fragShaderCode = readFile("shaders/frag.spv");

        // 创建着色器模块
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        // 顶点着色器阶段创建信息
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;   // 顶点着色器阶段
        vertShaderStageInfo.module = vertShaderModule;            // 顶点着色器模块
        vertShaderStageInfo.pName = "main";                       // 着色器入口函数名

        // 片段着色器阶段创建信息
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // 片段着色器阶段
        fragShaderStageInfo.module = fragShaderModule;            // 片段着色器模块
        fragShaderStageInfo.pName = "main";                       // 着色器入口函数名

        // 着色器阶段数组
        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // 顶点输入状态（这里没有顶点数据）
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

        // 输入装配状态（绘制三角形列表）
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;  // 绘制独立三角形
        inputAssembly.primitiveRestartEnable = VK_FALSE;               // 禁用图元重启

        // 视口状态（动态设置）
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;  // 一个视口
        viewportState.scissorCount = 1;   // 一个裁剪矩形

        // 光栅化状态
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;          // 禁用深度夹紧
        rasterizer.rasterizerDiscardEnable = VK_FALSE;   // 不丢弃片段
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;   // 填充多边形
        rasterizer.lineWidth = 1.0f;                     // 线宽为1像素
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;     // 背面剔除
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;  // 顺时针为正面
        rasterizer.depthBiasEnable = VK_FALSE;           // 禁用深度偏移

        // 多重采样状态
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;      // 禁用采样着色
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;  // 1个采样点

        // 颜色混合附件状态
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
                                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;  // 写入所有颜色通道
        colorBlendAttachment.blendEnable = VK_FALSE;  // 禁用颜色混合

        // 颜色混合状态
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;        // 禁用逻辑操作
        colorBlending.logicOp = VK_LOGIC_OP_COPY;      // 无逻辑操作
        colorBlending.attachmentCount = 1;             // 一个颜色混合附件
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;        // 混合常量
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        // 动态状态（视口和裁剪矩形将在运行时设置）
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // 管线布局创建信息
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;         // 没有描述符集布局
        pipelineLayoutInfo.pushConstantRangeCount = 0; // 没有推送常量

        // 创建管线布局
        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        // 图形管线创建信息
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;                  // 两个着色器阶段
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;         // 使用创建的管线布局
        pipelineInfo.renderPass = renderPass;         // 使用创建的渲染过程
        pipelineInfo.subpass = 0;                     // 在第一个子过程中使用

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // 不从现有管线派生

        // 创建图形管线
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        // 销毁着色器模块（已经链接到管线，不再需要）
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    // 创建帧缓冲区
    void createFramebuffers() {
        // 调整帧缓冲区向量大小以匹配图像视图数量
        swapChainFramebuffers.resize(swapChainImageViews.size());

        // 为每个图像视图创建帧缓冲区
        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            // 帧缓冲区附件（这里是交换链图像视图）
            VkImageView attachments[] = {
                swapChainImageViews[i]
            };

            // 帧缓冲区创建信息
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;      // 关联的渲染过程
            framebufferInfo.attachmentCount = 1;          // 一个附件
            framebufferInfo.pAttachments = attachments;   // 指向附件数组
            framebufferInfo.width = swapChainExtent.width;   // 宽度
            framebufferInfo.height = swapChainExtent.height; // 高度
            framebufferInfo.layers = 1;                   // 层数

            // 创建帧缓冲区
            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    // 创建命令池
    void createCommandPool() {
        // 查找支持图形操作的队列族
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        // 命令池创建信息
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;  // 允许单独重置命令缓冲区
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();  // 使用图形队列族

        // 创建命令池
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    // 创建命令缓冲区
    void createCommandBuffer() {
        // 命令缓冲区分配信息
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;                    // 从哪个命令池分配
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;      // 主命令缓冲区（可以直接提交到队列）
        allocInfo.commandBufferCount = 1;                       // 分配一个命令缓冲区

        // 分配命令缓冲区
        if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    // 记录命令缓冲区
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        // 命令缓冲区开始信息
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        // 开始记录命令缓冲区
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        // 渲染过程开始信息
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;                         // 使用的渲染过程
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex]; // 对应的帧缓冲区
        renderPassInfo.renderArea.offset = { 0, 0 };                    // 渲染区域起始位置
        renderPassInfo.renderArea.extent = swapChainExtent;             // 渲染区域大小

        // 清除颜色值（黑色不透明）
        VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        // 开始渲染过程
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // 绑定图形管线
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        // 设置视口
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        // 设置裁剪矩形
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // 绘制命令（绘制3个顶点，1个实例，从第0个顶点开始，第0个实例）
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        // 结束渲染过程
        vkCmdEndRenderPass(commandBuffer);

        // 结束命令缓冲区记录
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    // 创建同步对象
    void createSyncObjects() {
        // 信号量创建信息
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        // 围栏创建信息
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;  // 初始状态为已发出信号

        // 创建同步对象
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

    // 绘制一帧
    void drawFrame() {
        // 等待上一帧完成（围栏变为已发出信号）
        vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        // 重置围栏为未发出信号状态
        vkResetFences(device, 1, &inFlightFence);

        // 获取下一个可用的交换链图像索引
        uint32_t imageIndex;
        vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        // 重置命令缓冲区并重新记录
        vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffer, imageIndex);

        // 提交信息
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // 等待信号量和等待阶段
        VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        // 要提交的命令缓冲区
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // 发出信号的信号量
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        // 提交命令缓冲区到图形队列
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        // 呈现信息
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // 等待渲染完成的信号量
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        // 要呈现的交换链和图像索引
        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        // 将图像呈现到屏幕
        vkQueuePresentKHR(presentQueue, &presentInfo);
    }

    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}