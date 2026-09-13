#pragma once

#include "main_headers.h"

#if USED_GRAPHICS_API == VULKAN_API

#include "render_pass_descriptor.h"

namespace gb
{
    class vk_render_pass_descriptor : public i_render_pass_descriptor_impl
    {
    private:

        enum e_mode
        {
            e_mode_ws = 0,
            e_mode_ss,
            e_mode_output
        };

        e_mode m_mode = e_mode_ws;
        ui32 m_frame_width = 0;
        ui32 m_frame_height = 0;
        bool m_is_depth_load_enabled = false;

        VkRenderPass m_render_pass = VK_NULL_HANDLE;
        VkFramebuffer m_frame_buffer = VK_NULL_HANDLE;

        std::vector<VkImage> m_color_images;
        std::vector<VkDeviceMemory> m_color_images_memory;
        std::vector<VkImageView> m_color_images_view;
        std::vector<VkSampler> m_color_samplers;
        std::vector<VkClearValue> m_clear_values;

        struct depth_attachment_data
        {
            VkImage m_image = VK_NULL_HANDLE;
            VkDeviceMemory m_memory = VK_NULL_HANDLE;
            VkImageView m_image_view = VK_NULL_HANDLE;
            VkSampler m_sampler = VK_NULL_HANDLE;
            texture_shared_ptr m_texture = nullptr;

            ~depth_attachment_data();
        };

        std::shared_ptr<depth_attachment_data> m_depth_attachment = nullptr;
        static std::unordered_map<ui64, std::weak_ptr<depth_attachment_data>> m_depth_attachments_pool;

        std::vector<texture_shared_ptr> m_color_attachments_texture;

        static VkRenderPass m_current_render_pass;
        static ui32 m_current_color_attachments_count;

        void construct(const std::string& guid, ui32 width, ui32 height, const std::vector<std::shared_ptr<configuration>>& attachments_configurations, bool is_depth_load_enabled);
        void create_color_attachment(const std::string& guid, VkFormat format, ui32 width, ui32 height);
        void create_depth_attachment(const std::string& guid, ui32 width, ui32 height);

        static VkFormat get_pixel_format(ui32 format);

    public:

        vk_render_pass_descriptor() = default;
        ~vk_render_pass_descriptor();

        static std::shared_ptr<vk_render_pass_descriptor> construct_ws_render_pass_descriptor(const std::shared_ptr<ws_technique_configuration>& configuration);
        static std::shared_ptr<vk_render_pass_descriptor> construct_ss_render_pass_descriptor(const std::shared_ptr<ss_technique_configuration>& configuration);
        static std::shared_ptr<vk_render_pass_descriptor> construct_output_render_pass_descriptor(const std::string& name);

        static VkRenderPass get_current_render_pass();
        static ui32 get_current_color_attachments_count();

        void bind() override;
        void unbind() override;

        std::vector<texture_shared_ptr> get_color_attachments_texture() const override;
        texture_shared_ptr get_depth_attachment_texture() const override;
    };
};

#endif
