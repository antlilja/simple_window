#include "window.hpp"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>

#include <vulkan/vulkan.hpp>
#if defined(__linux__)
#    include <vulkan/vulkan_xcb.h>
#elif defined(_WIN32)
#    include <vulkan/vulkan_win32.h>
#endif

const std::vector<uint32_t> vert_spv = {
    0x07230203, 0x00010000, 0x00080007, 0x00000036, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0008000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x00000022, 0x00000026, 0x00000031,
    0x00030003, 0x00000002, 0x000001c2, 0x00090004, 0x415f4c47, 0x735f4252, 0x72617065, 0x5f657461,
    0x64616873, 0x6f5f7265, 0x63656a62, 0x00007374, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000,
    0x00050005, 0x0000000c, 0x69736f70, 0x6e6f6974, 0x00000073, 0x00040005, 0x00000017, 0x6f6c6f63,
    0x00007372, 0x00060005, 0x00000020, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006,
    0x00000020, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x00000020, 0x00000001,
    0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x00000020, 0x00000002, 0x435f6c67,
    0x4470696c, 0x61747369, 0x0065636e, 0x00070006, 0x00000020, 0x00000003, 0x435f6c67, 0x446c6c75,
    0x61747369, 0x0065636e, 0x00030005, 0x00000022, 0x00000000, 0x00060005, 0x00000026, 0x565f6c67,
    0x65747265, 0x646e4978, 0x00007865, 0x00050005, 0x00000031, 0x67617266, 0x6f6c6f43, 0x00000072,
    0x00050048, 0x00000020, 0x00000000, 0x0000000b, 0x00000000, 0x00050048, 0x00000020, 0x00000001,
    0x0000000b, 0x00000001, 0x00050048, 0x00000020, 0x00000002, 0x0000000b, 0x00000003, 0x00050048,
    0x00000020, 0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x00000020, 0x00000002, 0x00040047,
    0x00000026, 0x0000000b, 0x0000002a, 0x00040047, 0x00000031, 0x0000001e, 0x00000000, 0x00020013,
    0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017,
    0x00000007, 0x00000006, 0x00000002, 0x00040015, 0x00000008, 0x00000020, 0x00000000, 0x0004002b,
    0x00000008, 0x00000009, 0x00000003, 0x0004001c, 0x0000000a, 0x00000007, 0x00000009, 0x00040020,
    0x0000000b, 0x00000006, 0x0000000a, 0x0004003b, 0x0000000b, 0x0000000c, 0x00000006, 0x0004002b,
    0x00000006, 0x0000000d, 0x00000000, 0x0004002b, 0x00000006, 0x0000000e, 0xbf000000, 0x0005002c,
    0x00000007, 0x0000000f, 0x0000000d, 0x0000000e, 0x0004002b, 0x00000006, 0x00000010, 0x3f000000,
    0x0005002c, 0x00000007, 0x00000011, 0x00000010, 0x00000010, 0x0005002c, 0x00000007, 0x00000012,
    0x0000000e, 0x00000010, 0x0006002c, 0x0000000a, 0x00000013, 0x0000000f, 0x00000011, 0x00000012,
    0x00040017, 0x00000014, 0x00000006, 0x00000003, 0x0004001c, 0x00000015, 0x00000014, 0x00000009,
    0x00040020, 0x00000016, 0x00000006, 0x00000015, 0x0004003b, 0x00000016, 0x00000017, 0x00000006,
    0x0004002b, 0x00000006, 0x00000018, 0x3f800000, 0x0006002c, 0x00000014, 0x00000019, 0x00000018,
    0x0000000d, 0x0000000d, 0x0006002c, 0x00000014, 0x0000001a, 0x0000000d, 0x00000018, 0x0000000d,
    0x0006002c, 0x00000014, 0x0000001b, 0x0000000d, 0x0000000d, 0x00000018, 0x0006002c, 0x00000015,
    0x0000001c, 0x00000019, 0x0000001a, 0x0000001b, 0x00040017, 0x0000001d, 0x00000006, 0x00000004,
    0x0004002b, 0x00000008, 0x0000001e, 0x00000001, 0x0004001c, 0x0000001f, 0x00000006, 0x0000001e,
    0x0006001e, 0x00000020, 0x0000001d, 0x00000006, 0x0000001f, 0x0000001f, 0x00040020, 0x00000021,
    0x00000003, 0x00000020, 0x0004003b, 0x00000021, 0x00000022, 0x00000003, 0x00040015, 0x00000023,
    0x00000020, 0x00000001, 0x0004002b, 0x00000023, 0x00000024, 0x00000000, 0x00040020, 0x00000025,
    0x00000001, 0x00000023, 0x0004003b, 0x00000025, 0x00000026, 0x00000001, 0x00040020, 0x00000028,
    0x00000006, 0x00000007, 0x00040020, 0x0000002e, 0x00000003, 0x0000001d, 0x00040020, 0x00000030,
    0x00000003, 0x00000014, 0x0004003b, 0x00000030, 0x00000031, 0x00000003, 0x00040020, 0x00000033,
    0x00000006, 0x00000014, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
    0x00000005, 0x0003003e, 0x0000000c, 0x00000013, 0x0003003e, 0x00000017, 0x0000001c, 0x0004003d,
    0x00000023, 0x00000027, 0x00000026, 0x00050041, 0x00000028, 0x00000029, 0x0000000c, 0x00000027,
    0x0004003d, 0x00000007, 0x0000002a, 0x00000029, 0x00050051, 0x00000006, 0x0000002b, 0x0000002a,
    0x00000000, 0x00050051, 0x00000006, 0x0000002c, 0x0000002a, 0x00000001, 0x00070050, 0x0000001d,
    0x0000002d, 0x0000002b, 0x0000002c, 0x0000000d, 0x00000018, 0x00050041, 0x0000002e, 0x0000002f,
    0x00000022, 0x00000024, 0x0003003e, 0x0000002f, 0x0000002d, 0x0004003d, 0x00000023, 0x00000032,
    0x00000026, 0x00050041, 0x00000033, 0x00000034, 0x00000017, 0x00000032, 0x0004003d, 0x00000014,
    0x00000035, 0x00000034, 0x0003003e, 0x00000031, 0x00000035, 0x000100fd, 0x00010038};

const std::vector<uint32_t> frag_spv = {
    0x07230203, 0x00010000, 0x00080007, 0x00000013, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0007000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000c, 0x00030010,
    0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00090004, 0x415f4c47, 0x735f4252,
    0x72617065, 0x5f657461, 0x64616873, 0x6f5f7265, 0x63656a62, 0x00007374, 0x00040005, 0x00000004,
    0x6e69616d, 0x00000000, 0x00050005, 0x00000009, 0x4374756f, 0x726f6c6f, 0x00000000, 0x00050005,
    0x0000000c, 0x67617266, 0x6f6c6f43, 0x00000072, 0x00040047, 0x00000009, 0x0000001e, 0x00000000,
    0x00040047, 0x0000000c, 0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
    0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004,
    0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003,
    0x00040017, 0x0000000a, 0x00000006, 0x00000003, 0x00040020, 0x0000000b, 0x00000001, 0x0000000a,
    0x0004003b, 0x0000000b, 0x0000000c, 0x00000001, 0x0004002b, 0x00000006, 0x0000000e, 0x3f800000,
    0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d,
    0x0000000a, 0x0000000d, 0x0000000c, 0x00050051, 0x00000006, 0x0000000f, 0x0000000d, 0x00000000,
    0x00050051, 0x00000006, 0x00000010, 0x0000000d, 0x00000001, 0x00050051, 0x00000006, 0x00000011,
    0x0000000d, 0x00000002, 0x00070050, 0x00000007, 0x00000012, 0x0000000f, 0x00000010, 0x00000011,
    0x0000000e, 0x0003003e, 0x00000009, 0x00000012, 0x000100fd, 0x00010038};

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validation_layers = {"VK_LAYER_LUNARG_standard_validation"};

const std::vector<const char*> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
constexpr bool debug_mode = false;
#else
constexpr bool debug_mode = true;
#endif

struct QueueFamilyIndices {
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;

    bool complete() { return graphics.has_value() && present.has_value(); }
};

class vulkan_app {
public:
    vulkan_app() {
        create_instance();
        create_surface();
        pick_device();
        create_device();
        create_swapchain();
        create_image_views();
        create_render_pass();
        create_pipeline();
        create_framebuffers();
        create_command_pool();
        create_and_record_command_buffers();
        create_sync_objects();

        while (m_window.is_open()) {
            m_window.update();
            draw();
        }
        m_device.waitIdle();
    }

    ~vulkan_app() {
        destroy_window_dependet_resources();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_device.destroySemaphore(m_render_finished[i]);
            m_device.destroySemaphore(m_image_available[i]);
            m_device.destroyFence(m_in_flight[i]);
        }

        m_device.destroyCommandPool(m_command_pool);

        m_device.destroy();

        m_instance.destroySurfaceKHR(m_surface);
        m_instance.destroy();
    }

private:
    window m_window;

    vk::Instance m_instance;
    vk::SurfaceKHR m_surface;

    vk::PhysicalDevice m_physical_device;
    vk::Device m_device;

    vk::Queue m_graphics_queue;
    vk::Queue m_present_queue;

    vk::SwapchainKHR m_swapchain;
    std::vector<vk::Image> m_swapchain_images;
    vk::Format m_swapchain_format = vk::Format::eUndefined;
    vk::Extent2D m_swapchain_extent;
    std::vector<vk::ImageView> m_swapchain_image_views;
    std::vector<vk::Framebuffer> m_swapchain_framebuffers;

    vk::RenderPass m_render_pass;
    vk::PipelineLayout m_pipeline_layout;
    vk::Pipeline m_pipeline;

    vk::CommandPool m_command_pool;
    std::vector<vk::CommandBuffer> m_command_buffers;

    std::vector<vk::Semaphore> m_image_available;
    std::vector<vk::Semaphore> m_render_finished;
    std::vector<vk::Fence> m_in_flight;
    size_t m_current_frame = 0;

private:
    void destroy_window_dependet_resources() {
        m_device.waitForFences(m_in_flight, VK_TRUE, std::numeric_limits<uint64_t>::max());
        m_device.waitIdle();
        for (auto framebuffer : m_swapchain_framebuffers) {
            m_device.destroyFramebuffer(framebuffer);
        }

        m_device.freeCommandBuffers(m_command_pool, m_command_buffers);
        m_device.destroyPipeline(m_pipeline);
        m_device.destroyPipelineLayout(m_pipeline_layout);
        m_device.destroyRenderPass(m_render_pass);

        for (auto view : m_swapchain_image_views) {
            m_device.destroyImageView(view);
        }

        m_device.destroySwapchainKHR(m_swapchain);
    }

    void recreate_window_dependent_resources() {
        while (m_window.get_width() == 0 || m_window.get_height() == 0) {
            m_window.update();
        }

        destroy_window_dependet_resources();

        create_swapchain();
        create_image_views();
        create_render_pass();
        create_pipeline();
        create_framebuffers();
        create_and_record_command_buffers();
    }

    void create_instance() {
        if constexpr (debug_mode) {
            if (!check_validation_layer_support()) {
                throw std::runtime_error("Validation layers not supported");
            }
        }

        vk::ApplicationInfo app_info = {"Vulkan example", VK_MAKE_VERSION(1, 0, 0), "No Engine",
                                        VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_1};

        vk::InstanceCreateInfo create_info = {vk::InstanceCreateFlags(), &app_info};

        std::vector<const char*> extensions;
        extensions.reserve(debug_mode ? 3 : 2);
        extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(__linux__)
        extensions.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(_WIN32)
        extensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

#endif
        if constexpr (debug_mode) {
            extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        if constexpr (debug_mode) {
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();
        }

        m_instance = vk::createInstance(create_info);
    }

    void create_surface() {
        VkSurfaceKHR surface;
#if defined(__linux__)
        VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.connection = m_window.get_connection();
        surfaceCreateInfo.window = m_window.get_window();
        if (vkCreateXcbSurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &surface) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to create surface");
        }
#elif defined(_WIN32)
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.hinstance = m_window.get_hinstance();
        surfaceCreateInfo.hwnd = m_window.get_hwnd();
        if (vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &surface) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to create surface");
        }
#endif
        m_surface = vk::SurfaceKHR(surface);
    }

    void pick_device() {
        auto devices = m_instance.enumeratePhysicalDevices();

        for (auto device : devices) {
            if (is_device_suitable(device)) {
                m_physical_device = device;
                break;
            }
        }

        if (m_physical_device == vk::PhysicalDevice()) {
            throw std::runtime_error("Failed to find a physical device");
        }
    }

    void create_device() {
        auto indices = find_queue_families(m_physical_device);

        std::set<uint32_t> unique_queue_families = {indices.graphics.value(),
                                                    indices.present.value()};
        float queue_priority = 1.0f;

        std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
        for (uint32_t queue_family : unique_queue_families) {
            vk::DeviceQueueCreateInfo create_info = {vk::DeviceQueueCreateFlags(), queue_family, 1,
                                                     &queue_priority};
            queue_create_infos.push_back(create_info);
        }

        vk::DeviceCreateInfo create_info = {vk::DeviceCreateFlags(),
                                            static_cast<uint32_t>(queue_create_infos.size()),
                                            queue_create_infos.data(),
                                            0,
                                            nullptr,
                                            static_cast<uint32_t>(device_extensions.size()),
                                            device_extensions.data()};

        if constexpr (debug_mode) {
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();
        }
        else {
            create_info.enabledLayerCount = 0;
        }

        m_device = m_physical_device.createDevice(create_info);

        m_graphics_queue = m_device.getQueue(indices.graphics.value(), 0);
        m_present_queue = m_device.getQueue(indices.present.value(), 0);
    }

    void create_swapchain() {
        auto formats = m_physical_device.getSurfaceFormatsKHR(m_surface);
        auto present_modes = m_physical_device.getSurfacePresentModesKHR(m_surface);
        auto capabilities = m_physical_device.getSurfaceCapabilitiesKHR(m_surface);

        auto surface_format = choose_swapchain_surface_format(formats);
        auto present_mode = choose_swapchain_present_mode(present_modes);
        auto extent = choose_swapchain_extent(capabilities);

        uint32_t image_count = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
            image_count = capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR create_info = {vk::SwapchainCreateFlagsKHR(),
                                                  m_surface,
                                                  image_count,
                                                  surface_format.format,
                                                  surface_format.colorSpace,
                                                  extent,
                                                  1,
                                                  vk::ImageUsageFlagBits::eColorAttachment,
                                                  vk::SharingMode::eExclusive,
                                                  0,
                                                  nullptr,
                                                  capabilities.currentTransform,
                                                  vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                                  present_mode,
                                                  VK_TRUE};

        auto indices = find_queue_families(m_physical_device);
        uint32_t queue_family_indices[] = {indices.graphics.value(), indices.present.value()};

        if (indices.graphics != indices.present) {
            create_info.imageSharingMode = vk::SharingMode::eConcurrent;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = queue_family_indices;
        }

        m_swapchain = m_device.createSwapchainKHR(create_info);

        m_swapchain_images = m_device.getSwapchainImagesKHR(m_swapchain);

        m_swapchain_format = surface_format.format;
        m_swapchain_extent = extent;
    }

    void create_image_views() {
        m_swapchain_image_views.clear();
        m_swapchain_image_views.reserve(m_swapchain_images.size());

        for (auto image : m_swapchain_images) {
            vk::ImageViewCreateInfo create_info = {
                vk::ImageViewCreateFlags(),
                image,
                vk::ImageViewType::e2D,
                m_swapchain_format,
                vk::ComponentMapping(),
                vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)};
            m_swapchain_image_views.emplace_back(m_device.createImageView(create_info));
        }
    }

    void create_render_pass() {
        vk::AttachmentDescription color = {
            vk::AttachmentDescriptionFlags(), m_swapchain_format,
            vk::SampleCountFlagBits::e1,      vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,    vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
            vk::ImageLayout::ePresentSrcKHR};

        vk::AttachmentReference color_ref = {0, vk::ImageLayout::eColorAttachmentOptimal};

        vk::SubpassDescription subpass = {vk::SubpassDescriptionFlags(),
                                          vk::PipelineBindPoint::eGraphics,
                                          0,
                                          nullptr,
                                          1,
                                          &color_ref};

        vk::SubpassDependency subpass_dependency = {
            VK_SUBPASS_EXTERNAL,
            0,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::AccessFlags(),
            vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
            vk::DependencyFlags()};

        std::array<vk::AttachmentDescription, 1> attach = {color};
        vk::RenderPassCreateInfo create_info = {vk::RenderPassCreateFlags(),
                                                static_cast<uint32_t>(attach.size()),
                                                attach.data(),
                                                1,
                                                &subpass,
                                                1,
                                                &subpass_dependency};

        m_render_pass = m_device.createRenderPass(create_info);
    }

    void create_pipeline() {
        auto vert_module = create_shader_module(vert_spv);
        auto frag_module = create_shader_module(frag_spv);

        std::array<vk::PipelineShaderStageCreateInfo, 2> stages = {
            {{vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vert_module,
              "main", nullptr},
             {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, frag_module,
              "main", nullptr}}};

        vk::PipelineVertexInputStateCreateInfo vertex_info = {};

        vk::PipelineInputAssemblyStateCreateInfo input_assembly = {
            vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList,
            VK_FALSE};

        vk::Viewport viewport = {0.0f,
                                 0.0f,
                                 static_cast<float>(m_swapchain_extent.width),
                                 static_cast<float>(m_swapchain_extent.height),
                                 0.0f,
                                 1.0f};

        vk::Rect2D scissor = {{0, 0}, m_swapchain_extent};

        vk::PipelineViewportStateCreateInfo viewport_info = {vk::PipelineViewportStateCreateFlags(),
                                                             1, &viewport, 1, &scissor};

        vk::PipelineRasterizationStateCreateInfo raster_info = {
            vk::PipelineRasterizationStateCreateFlags(),
            VK_FALSE,
            VK_FALSE,
            vk::PolygonMode::eFill,
            vk::CullModeFlagBits::eBack,
            vk::FrontFace::eClockwise,
            VK_FALSE,
            0.0f,
            0.0f,
            0.0f,
            1.0f};

        vk::PipelineMultisampleStateCreateInfo multisampling = {};

        vk::PipelineColorBlendAttachmentState color_attach = {};
        color_attach.colorWriteMask =
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

        vk::PipelineColorBlendStateCreateInfo blend_info = {};
        blend_info.attachmentCount = 1;
        blend_info.pAttachments = &color_attach;

        vk::PipelineLayoutCreateInfo pipeline_create_info = {};

        m_pipeline_layout = m_device.createPipelineLayout(pipeline_create_info);

        vk::GraphicsPipelineCreateInfo create_info = {vk::PipelineCreateFlags(),
                                                      static_cast<uint32_t>(stages.size()),
                                                      stages.data(),
                                                      &vertex_info,
                                                      &input_assembly,
                                                      nullptr,
                                                      &viewport_info,
                                                      &raster_info,
                                                      &multisampling,
                                                      nullptr,
                                                      &blend_info,
                                                      nullptr,
                                                      m_pipeline_layout,
                                                      m_render_pass};

        m_pipeline = m_device.createGraphicsPipeline(vk::PipelineCache(), create_info);

        m_device.destroyShaderModule(frag_module);
        m_device.destroyShaderModule(vert_module);
    }

    void create_framebuffers() {
        m_swapchain_framebuffers.clear();
        m_swapchain_framebuffers.reserve(m_swapchain_image_views.size());

        for (auto view : m_swapchain_image_views) {
            vk::FramebufferCreateInfo create_info = {vk::FramebufferCreateFlags(),
                                                     m_render_pass,
                                                     1,
                                                     &view,
                                                     m_swapchain_extent.width,
                                                     m_swapchain_extent.height,
                                                     1};

            m_swapchain_framebuffers.emplace_back(m_device.createFramebuffer(create_info));
        }
    }

    void create_command_pool() {
        auto indices = find_queue_families(m_physical_device);

        vk::CommandPoolCreateInfo create_info = {vk::CommandPoolCreateFlags(),
                                                 indices.graphics.value()};
        m_command_pool = m_device.createCommandPool(create_info);
    }

    void create_and_record_command_buffers() {
        m_command_buffers.clear();
        m_command_buffers.reserve(m_swapchain_framebuffers.size());

        vk::CommandBufferAllocateInfo alloc_info = {
            m_command_pool, vk::CommandBufferLevel::ePrimary,
            static_cast<uint32_t>(m_swapchain_framebuffers.size())};

        m_command_buffers = m_device.allocateCommandBuffers(alloc_info);

        for (size_t i = 0; i < m_command_buffers.size(); i++) {
            vk::CommandBufferBeginInfo begin_info = {
                vk::CommandBufferUsageFlagBits::eSimultaneousUse};

            m_command_buffers[i].begin(begin_info);

            std::array<float, 4> clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
            vk::ClearValue clear_value = {clear_color};

            vk::RenderPassBeginInfo render_pass_info = {m_render_pass, m_swapchain_framebuffers[i],
                                                        vk::Rect2D({0, 0}, m_swapchain_extent), 1,
                                                        &clear_value};

            m_command_buffers[i].beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
            m_command_buffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
            m_command_buffers[i].draw(3, 1, 0, 0);
            m_command_buffers[i].endRenderPass();
            m_command_buffers[i].end();
        }
    }

    void create_sync_objects() {
        m_image_available.resize(MAX_FRAMES_IN_FLIGHT);
        m_render_finished.resize(MAX_FRAMES_IN_FLIGHT);
        m_in_flight.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_image_available[i] = m_device.createSemaphore(vk::SemaphoreCreateInfo());
            m_render_finished[i] = m_device.createSemaphore(vk::SemaphoreCreateInfo());
            m_in_flight[i] =
                m_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
        }
    }

    void draw() {
        m_device.waitForFences(m_in_flight[m_current_frame], VK_TRUE,
                               std::numeric_limits<uint64_t>::max());

        auto [aquire_result, image_index] =
            m_device.acquireNextImageKHR(m_swapchain, std::numeric_limits<uint64_t>::max(),
                                         m_image_available[m_current_frame], vk::Fence());

        if (aquire_result == vk::Result::eErrorOutOfDateKHR ||
            aquire_result == vk::Result::eSuboptimalKHR ||
            aquire_result == vk::Result::eErrorIncompatibleDisplayKHR) {
            recreate_window_dependent_resources();
            return;
        }

        vk::PipelineStageFlags wait_stages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        vk::SubmitInfo submit_info = {1,
                                      &m_image_available[m_current_frame],
                                      &wait_stages,
                                      1,
                                      &m_command_buffers[image_index],
                                      1,
                                      &m_render_finished[m_current_frame]};

        m_device.resetFences(m_in_flight[m_current_frame]);

        m_graphics_queue.submit(submit_info, m_in_flight[m_current_frame]);

        vk::PresentInfoKHR present_info = {1, &m_render_finished[m_current_frame], 1, &m_swapchain,
                                           &image_index};

        auto present_result = m_present_queue.presentKHR(&present_info);

        if (present_result == vk::Result::eErrorOutOfDateKHR ||
            present_result == vk::Result::eSuboptimalKHR || m_window.should_resize()) {
            recreate_window_dependent_resources();
        }

        m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    vk::ShaderModule create_shader_module(const std::vector<uint32_t>& code) {
        vk::ShaderModuleCreateInfo create_info = {vk::ShaderModuleCreateFlags(),
                                                  code.size() * sizeof(uint32_t), code.data()};

        return m_device.createShaderModule(create_info);
    }

    vk::SurfaceFormatKHR
    choose_swapchain_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats) {
        if (available_formats.size() == 1 &&
            available_formats[0].format == vk::Format::eUndefined) {
            vk::SurfaceFormatKHR format;
            format.format = vk::Format::eB8G8R8A8Unorm;
            format.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
            return format;
        }

        for (const auto& availableFormat : available_formats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
                availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }

        return available_formats[0];
    }

    vk::PresentModeKHR
    choose_swapchain_present_mode(const std::vector<vk::PresentModeKHR>& available_present_modes) {
        auto best_mode = vk::PresentModeKHR::eFifo;

        for (const auto& present_mode : available_present_modes) {
            if (present_mode == vk::PresentModeKHR::eMailbox) {
                return present_mode;
            }
            else if (present_mode == vk::PresentModeKHR::eImmediate) {
                best_mode = present_mode;
            }
        }

        return best_mode;
    }

    vk::Extent2D choose_swapchain_extent(const vk::SurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {

            vk::Extent2D actual_extent = {m_window.get_width(), m_window.get_height()};

            actual_extent.width =
                std::max(capabilities.minImageExtent.width,
                         std::min(capabilities.maxImageExtent.width, actual_extent.width));
            actual_extent.height =
                std::max(capabilities.minImageExtent.height,
                         std::min(capabilities.maxImageExtent.height, actual_extent.height));

            return actual_extent;
        }
    }

    bool is_device_suitable(vk::PhysicalDevice device) {
        return find_queue_families(device).complete() && check_device_extension_support(device) &&
               !device.getSurfaceFormatsKHR(m_surface).empty() &&
               !device.getSurfacePresentModesKHR(m_surface).empty();
    }

    bool check_device_extension_support(vk::PhysicalDevice device) {
        auto available_extensions = device.enumerateDeviceExtensionProperties();

        std::set<std::string> required_extensions(device_extensions.begin(),
                                                  device_extensions.end());

        for (const auto& extension : available_extensions) {
            required_extensions.erase(extension.extensionName);
        }

        return required_extensions.empty();
    }

    QueueFamilyIndices find_queue_families(vk::PhysicalDevice device) {
        QueueFamilyIndices indices;

        auto queue_families = device.getQueueFamilyProperties();

        int i = 0;
        for (const auto& queue_family : queue_families) {
            if (queue_family.queueCount > 0 &&
                queue_family.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.graphics = i;
            }

            auto present_support = device.getSurfaceSupportKHR(i, m_surface);

            if (queue_family.queueCount > 0 && present_support) {
                indices.present = i;
            }

            if (indices.complete()) break;

            i++;
        }

        return indices;
    }

    bool check_validation_layer_support() {
        const auto available_layers = vk::enumerateInstanceLayerProperties();

        for (const char* layerName : validation_layers) {
            bool layerFound = false;

            for (const auto& layerProperties : available_layers) {
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
};

int main() {
    try {
        vulkan_app app;
    } catch (std::exception& e) {
        std::cerr << e.what();
    }
}