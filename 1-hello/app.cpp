//
// Created by loulfy on 01/06/17.
//

#include "app.hpp"

Application::Application(const uint w, const uint h) : m_finx(0), m_qinx(0), m_debug_report_callback(nullptr)
{
    static plog::ColorConsoleAppender<Report> console;
    plog::init(plog::verbose, &console);
    LOGI << "INIT LOGGER";

    // Initialize GLFW 3
    glfwInit();
    glfwSetErrorCallback(Application::glfw_report);

    // Check Vulkan Driver
    if(glfwVulkanSupported()) LOGI << "Vulkan Supported";
    else
    {
        LOGE << "Vulkan NOT supported";
        throw std::runtime_error("Vulkan NOT supported");
    }

    // Create window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(w, h, "vkyx", NULL, NULL);

    // Create vk::Instance
    this->handshake();
    // Create Debug Report
    this->debug();
    // Create vk::SurfaceKHR
    this->surface();
    // Find queue family & Create vk::Device
    this->presentation();
}

Application::~Application()
{
    m_instance.destroySurfaceKHR(m_surface);
    m_device.destroy();
    if(m_debug_report_callback)
    {
        auto destroy_dr = (PFN_vkDestroyDebugReportCallbackEXT) m_instance.getProcAddr("vkDestroyDebugReportCallbackEXT");
        destroy_dr(m_instance, m_debug_report_callback, nullptr);
    }
    glfwDestroyWindow(m_window);
    glfwTerminate();
    m_instance.destroy();
}

void Application::run()
{
    while(!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
    }
}

void Application::handshake()
{
    // Instance's extensions
    auto installedExtensions = vk::enumerateInstanceExtensionProperties();
    std::vector<const char*> wantedExtensions =
    {
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME
    };
    auto extensions = std::vector<const char*>();

    // SurfaceKHR extensions
    uint32_t count;
    auto required = glfwGetRequiredInstanceExtensions(&count);
    for(uint32_t i=0; i < count; i++)
    {
        wantedExtensions.push_back(required[i]);
    }

    // Pick extensions
    for(auto &w : wantedExtensions)
    {
        for(auto &i : installedExtensions)
        {
            if(std::string(i.extensionName).compare(w) == 0)
            {
                LOGI << w;
                extensions.emplace_back(w);
                break;
            }
        }
    }

    // Instance's layers
    auto installedLayers = vk::enumerateInstanceLayerProperties();
    std::vector<const char*> wantedLayers =
    {
        "VK_LAYER_GOOGLE_threading",
        "VK_LAYER_LUNARG_parameter_validation",
        "VK_LAYER_LUNARG_object_tracker",
        "VK_LAYER_LUNARG_core_validation",
        "VK_LAYER_LUNARG_swapchain",
        "VK_LAYER_GOOGLE_unique_objects"
    };
    auto layers = std::vector<const char*>();

    // Pick layers
    for(auto &w : wantedLayers)
    {
        for(auto &i : installedLayers)
        {
            if(std::string(i.layerName).compare(w) == 0)
            {
                LOGI << w;
                layers.emplace_back(w);
                break;
            }
        }
    }

    m_appi.pApplicationName = "hello";
    m_appi.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    m_appi.pEngineName = "vkyx";
    m_appi.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    m_appi.apiVersion = VK_API_VERSION_1_0;

    auto ici = vk::InstanceCreateInfo();
    ici.flags = vk::InstanceCreateFlags();
    ici.pApplicationInfo = &m_appi;
    ici.enabledLayerCount = static_cast<uint32_t>(layers.size());
    ici.ppEnabledLayerNames = layers.data();
    ici.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    ici.ppEnabledExtensionNames = extensions.data();

    m_instance = vk::createInstance(ici);
}

void Application::debug()
{
    auto create_dr = (PFN_vkCreateDebugReportCallbackEXT) m_instance.getProcAddr("vkCreateDebugReportCallbackEXT");

    VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
    callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    callbackCreateInfo.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    callbackCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT) Application::debug_report;

    create_dr(m_instance, &callbackCreateInfo, nullptr, &m_debug_report_callback);
}

void Application::glfw_report(int error, const char* description)
{
    LOGE << description;
}

vk::Bool32 Application::debug_report(VkDebugReportFlagsEXT msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData)
{
    char buf[512];
    snprintf(buf,sizeof(buf),"[%s] Code %d : %s", pLayerPrefix, msgCode, pMsg);
    switch(msgFlags)
    {
        case (VkDebugReportFlagsEXT) vk::DebugReportFlagBitsEXT::eInformation        : LOGI << buf;  return VK_FALSE;  // 1
        case (VkDebugReportFlagsEXT) vk::DebugReportFlagBitsEXT::eWarning            : LOGW << buf;  return VK_FALSE;  // 2
        case (VkDebugReportFlagsEXT) vk::DebugReportFlagBitsEXT::ePerformanceWarning : LOGV << buf;  return VK_FALSE;  // 4
        case (VkDebugReportFlagsEXT) vk::DebugReportFlagBitsEXT::eError              : LOGE << buf;  return VK_TRUE;   // 8 Bail out for errors
        case (VkDebugReportFlagsEXT) vk::DebugReportFlagBitsEXT::eDebug              : LOGD << buf;  return VK_FALSE;  //16
        default : return VK_FALSE; //Don't bail out.
    }
}

void Application::surface()
{
    auto vksurface = VkSurfaceKHR();
    VkResult err = glfwCreateWindowSurface(m_instance, m_window, nullptr, &vksurface);
    if(err)
    {
        LOGE << "VK surface creation failed";
    }
    m_surface = vk::SurfaceKHR(vksurface);
}

void Application::presentation()
{
    auto physicalDevices = m_instance.enumeratePhysicalDevices();

    // Rate the physical devices
    std::sort(physicalDevices.begin(), physicalDevices.end());
    auto gpu = physicalDevices[0];

    // Device Properties & Layers
    auto p = gpu.getProperties();
    const std::map<vk::PhysicalDeviceType, std::string> devTypes
    {
        {vk::PhysicalDeviceType::eOther, "OTHER"},
        {vk::PhysicalDeviceType::eIntegratedGpu, "INTEGRATED"},
        {vk::PhysicalDeviceType::eDiscreteGpu, "DISCRETE"},
        {vk::PhysicalDeviceType::eVirtualGpu, "VIRTUAL"},
        {vk::PhysicalDeviceType::eCpu, "CPU"}
    };

    auto deviceExtensions = std::vector<const char*>();
    auto deviceValidationLayers = std::vector<const char*>();

    // Memory Properties
    auto m = gpu.getMemoryProperties();
    auto heaps = std::vector<vk::MemoryHeap>(m.memoryHeaps, m.memoryHeaps + m.memoryHeapCount);
    for(auto h : heaps)
    {
        if(h.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
        {
            LOGI << "GPU " << devTypes.at(p.deviceType) << " : " << p.deviceName << " : VRAM " << static_cast<float>(h.size/1000000000) << " Go";
        }
    }

    // Queue Families Properties
    auto gpuQueueProps = gpu.getQueueFamilyProperties();

    float priority = 0.0;
    auto qci = std::vector<vk::DeviceQueueCreateInfo>();

    for(auto& queue_family : gpuQueueProps)
    {
        std::string flags = "[ ";
        flags+= queue_family.queueFlags & vk::QueueFlagBits::eCompute ? "COMPUTE " : "";
        flags+= queue_family.queueFlags & vk::QueueFlagBits::eGraphics ? "GRAPHICS " : "";
        flags+= queue_family.queueFlags & vk::QueueFlagBits::eTransfer ? "TRANSFER " : "";
        flags+= queue_family.queueFlags & vk::QueueFlagBits::eSparseBinding ? "SPARSE " : "";
        flags+= "]";
        LOGI << "Queue-family:" << m_finx << " count:" << std::setfill('0') << std::setw(2) << queue_family.queueCount << " flags:" << flags;
        if(queue_family.queueFlags & vk::QueueFlagBits::eGraphics && gpu.getSurfaceSupportKHR(m_finx, m_surface))
        {
            // Create a single graphics queue.
            qci.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), m_finx, 1, &priority));
            break;
        }
        m_finx++;
    }

    // Device Features
    m_features = gpu.getFeatures();

    auto dci = vk::DeviceCreateInfo();
    dci.flags = vk::DeviceCreateFlags();
    dci.queueCreateInfoCount = static_cast<uint32_t>(qci.size());
    dci.pQueueCreateInfos = qci.data();
    dci.enabledLayerCount = static_cast<uint32_t>(deviceValidationLayers.size());
    dci.ppEnabledLayerNames = deviceValidationLayers.data();
    dci.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    dci.ppEnabledExtensionNames = deviceExtensions.data();
    dci.pEnabledFeatures = &m_features;

    // Device Creation
    m_device = gpu.createDevice(dci);
}