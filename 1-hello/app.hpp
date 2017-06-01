//
// Created by loulfy on 01/06/17.
//

#ifndef VKYX_APP_HPP
#define VKYX_APP_HPP

#include <plog/Appenders/ColorConsoleAppender.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <plog/Log.h>

#include <map>

class Application
{
    public:

        Application(const uint w, const uint h);
        ~Application();
        void run();

    private:

        // Callbacks
        static void glfw_report(int error, const char* description);
        static vk::Bool32 debug_report(VkDebugReportFlagsEXT msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData);

        // Vulkan Steps
        void handshake();
        void debug();
        void surface();
        void presentation();

        // Variables
        GLFWwindow* m_window;

        VkDebugReportCallbackEXT m_debug_report_callback;

        uint32_t m_finx;
        uint32_t m_qinx;

        vk::ApplicationInfo m_appi;
        vk::Instance m_instance;
        vk::SurfaceKHR m_surface;
        vk::Device m_device;
        vk::PhysicalDeviceFeatures m_features;
};

class Report
{
    public:

        static plog::util::nstring header()
        {
            return plog::util::nstring();
        }

        static plog::util::nstring format(const plog::Record& record)
        {
            plog::util::nstringstream ss;
            switch(record.getSeverity())
            {
                case plog::info:
                    ss << "[INFO] ";
                    break;
                case plog::warning:
                    ss << "[WARN] ";
                    break;
                case plog::error:
                    ss << "[ERRR] ";
                    break;
                default:
                    break;
            }

            ss << record.getMessage() << std::endl;

            return ss.str();
        }
};

#endif //VKYX_APP_HPP
