#include "vk_render_pass_descriptor.h"

#if USED_GRAPHICS_API == VULKAN_API

#include "attachment_configuration.h"
#include "ss_technique_configuration.h"
#include "texture.h"
#include "vk_device.h"
#include "vk_swap_chain.h"
#include "vk_utils.h"
#include "ws_technique_configuration.h"

namespace gb
{
    VkRenderPass vk_render_pass_descriptor::m_current_render_pass = VK_NULL_HANDLE;
    ui32 vk_render_pass_descriptor::m_current_color_attachments_count = 0;
    std::unordered_map<ui64, std::weak_ptr<vk_render_pass_descriptor::depth_attachment_data>> vk_render_pass_descriptor::m_depth_attachments_pool;

    vk_render_pass_descriptor::depth_attachment_data::~depth_attachment_data()
    {
        const auto device = vk_device::get_instance()->get_logical_device();
        if (device != VK_NULL_HANDLE)
        {
            if (m_sampler)
            {
                vkDestroySampler(device, m_sampler, nullptr);
            }
            if (m_image_view)
            {
                vkDestroyImageView(device, m_image_view, nullptr);
            }
            if (m_image)
            {
                vkDestroyImage(device, m_image, nullptr);
            }
            if (m_memory)
            {
                vkFreeMemory(device, m_memory, nullptr);
            }
        }
    }

    std::shared_ptr<vk_render_pass_descriptor> vk_render_pass_descriptor::construct_ws_render_pass_descriptor(const std::shared_ptr<ws_technique_configuration>& configuration)
    {
        const auto render_pass_descriptor = std::make_shared<vk_render_pass_descriptor>();
        render_pass_descriptor->m_mode = e_mode_ws;
        render_pass_descriptor->construct(configuration->get_guid(), configuration->get_frame_width(), configuration->get_frame_height(), configuration->get_attachments_configurations(), configuration->get_is_depth_compare_mode_enabled());
        return render_pass_descriptor;
    }

    std::shared_ptr<vk_render_pass_descriptor> vk_render_pass_descriptor::construct_ss_render_pass_descriptor(const std::shared_ptr<ss_technique_configuration>& configuration)
    {
        const auto render_pass_descriptor = std::make_shared<vk_render_pass_descriptor>();
        render_pass_descriptor->m_mode = e_mode_ss;
        render_pass_descriptor->construct(configuration->get_guid(), configuration->get_frame_width(), configuration->get_frame_height(), configuration->get_attachments_configurations(), false);
        return render_pass_descriptor;
    }

    std::shared_ptr<vk_render_pass_descriptor> vk_render_pass_descriptor::construct_output_render_pass_descriptor(const std::string& name)
    {
        const auto render_pass_descriptor = std::make_shared<vk_render_pass_descriptor>();
        const auto extent = vk_swap_chain::get_instance()->get_swap_chain_extent();
        render_pass_descriptor->m_mode = e_mode_output;
        render_pass_descriptor->m_frame_width = extent.width;
        render_pass_descriptor->m_frame_height = extent.height;
        render_pass_descriptor->m_render_pass = vk_swap_chain::get_instance()->get_render_pass();
        return render_pass_descriptor;
    }

    vk_render_pass_descriptor::~vk_render_pass_descriptor()
    {
        if (m_mode == e_mode_output)
        {
            return;
        }

        const auto device = vk_device::get_instance()->get_logical_device();
        if (device == VK_NULL_HANDLE)
        {
            return;
        }

        if (m_frame_buffer)
        {
            vkDestroyFramebuffer(device, m_frame_buffer, nullptr);
        }
        if (m_render_pass)
        {
            vkDestroyRenderPass(device, m_render_pass, nullptr);
        }
        for (auto sampler : m_color_samplers)
        {
            vkDestroySampler(device, sampler, nullptr);
        }
        for (auto image_view : m_color_images_view)
        {
            vkDestroyImageView(device, image_view, nullptr);
        }
        for (auto image : m_color_images)
        {
            vkDestroyImage(device, image, nullptr);
        }
        for (auto memory : m_color_images_memory)
        {
            vkFreeMemory(device, memory, nullptr);
        }
    }

    void vk_render_pass_descriptor::construct(const std::string& guid, ui32 width, ui32 height, const std::vector<std::shared_ptr<configuration>>& attachments_configurations, bool is_depth_load_enabled)
    {
        assert(!attachments_configurations.empty());
        m_frame_width = width;
        m_frame_height = height;
        m_is_depth_load_enabled = is_depth_load_enabled;

        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkAttachmentReference> color_attachment_references;
        std::vector<VkImageView> attachment_views;

        ui32 attachment_index = 0;
        for (const auto& attachment_configuration_it : attachments_configurations)
        {
            const auto attachment_configuration = std::static_pointer_cast<gb::attachment_configuration>(attachment_configuration_it);
            const auto format = get_pixel_format(attachment_configuration->get_pixel_format());
            const auto attachment_guid = guid + "." + attachment_configuration->get_name();
            create_color_attachment(attachment_guid, format, width, height);

            VkAttachmentDescription attachment = {};
            attachment.format = format;
            attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            attachments.push_back(attachment);

            VkAttachmentReference attachment_reference = {};
            attachment_reference.attachment = attachment_index++;
            attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            color_attachment_references.push_back(attachment_reference);
            attachment_views.push_back(m_color_images_view.back());

            VkClearValue clear_value = {};
            clear_value.color = { {
                attachment_configuration->get_clear_color_r(),
                attachment_configuration->get_clear_color_g(),
                attachment_configuration->get_clear_color_b(),
                attachment_configuration->get_clear_color_a()
            } };
            m_clear_values.push_back(clear_value);
        }

        create_depth_attachment(guid + ".depth", width, height);
        VkFormat depth_format;
        VkBool32 is_depth_format_supported = vk_device::get_supported_depth_format(&depth_format);
        assert(is_depth_format_supported);

        VkAttachmentDescription depth_attachment = {};
        depth_attachment.format = depth_format;
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = m_is_depth_load_enabled ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment.stencilLoadOp = m_is_depth_load_enabled ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments.push_back(depth_attachment);

        VkAttachmentReference depth_attachment_reference = {};
        depth_attachment_reference.attachment = attachment_index;
        depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachment_views.push_back(m_depth_attachment->m_image_view);

        VkSubpassDescription subpass_description = {};
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.colorAttachmentCount = static_cast<ui32>(color_attachment_references.size());
        subpass_description.pColorAttachments = color_attachment_references.data();
        subpass_description.pDepthStencilAttachment = &depth_attachment_reference;

        std::array<VkSubpassDependency, 2> dependencies = {};
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo render_pass_create_info = {};
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.attachmentCount = static_cast<ui32>(attachments.size());
        render_pass_create_info.pAttachments = attachments.data();
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass_description;
        render_pass_create_info.dependencyCount = static_cast<ui32>(dependencies.size());
        render_pass_create_info.pDependencies = dependencies.data();

        const auto device = vk_device::get_instance()->get_logical_device();
        VkResult result = vkCreateRenderPass(device, &render_pass_create_info, nullptr, &m_render_pass);
        assert(result == VK_SUCCESS);

        VkFramebufferCreateInfo frame_buffer_create_info = {};
        frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frame_buffer_create_info.renderPass = m_render_pass;
        frame_buffer_create_info.attachmentCount = static_cast<ui32>(attachment_views.size());
        frame_buffer_create_info.pAttachments = attachment_views.data();
        frame_buffer_create_info.width = width;
        frame_buffer_create_info.height = height;
        frame_buffer_create_info.layers = 1;
        result = vkCreateFramebuffer(device, &frame_buffer_create_info, nullptr, &m_frame_buffer);
        assert(result == VK_SUCCESS);

        VkClearValue depth_clear_value = {};
        depth_clear_value.depthStencil = { 1.f, 0 };
        m_clear_values.push_back(depth_clear_value);
    }

    void vk_render_pass_descriptor::create_color_attachment(const std::string& guid, VkFormat format, ui32 width, ui32 height)
    {
        const auto device = vk_device::get_instance()->get_logical_device();
        VkImageCreateInfo image_create_info = {};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = format;
        image_create_info.extent = { width, height, 1 };
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImage image = VK_NULL_HANDLE;
        VkResult result = vkCreateImage(device, &image_create_info, nullptr, &image);
        assert(result == VK_SUCCESS);

        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(device, image, &memory_requirements);
        VkMemoryAllocateInfo memory_allocate_info = {};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = vk_device::get_instance()->get_memory_type(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VkDeviceMemory memory = VK_NULL_HANDLE;
        result = vkAllocateMemory(device, &memory_allocate_info, nullptr, &memory);
        assert(result == VK_SUCCESS);
        result = vkBindImageMemory(device, image, memory, 0);
        assert(result == VK_SUCCESS);

        VkImageViewCreateInfo image_view_create_info = {};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = image;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = format;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.layerCount = 1;
        VkImageView image_view = VK_NULL_HANDLE;
        result = vkCreateImageView(device, &image_view_create_info, nullptr, &image_view);
        assert(result == VK_SUCCESS);

        VkSamplerCreateInfo sampler_create_info = {};
        sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_create_info.magFilter = VK_FILTER_LINEAR;
        sampler_create_info.minFilter = VK_FILTER_LINEAR;
        sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_create_info.maxLod = 1.f;
        VkSampler sampler = VK_NULL_HANDLE;
        result = vkCreateSampler(device, &sampler_create_info, nullptr, &sampler);
        assert(result == VK_SUCCESS);

		VkCommandBuffer command_buffer = vk_utils::create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		VkImageSubresourceRange subresource_range = {};
		subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource_range.levelCount = 1;
		subresource_range.layerCount = 1;
		vk_device::get_instance()->set_image_layout(command_buffer, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
												subresource_range, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		vk_utils::flush_command_buffer(command_buffer, vk_device::get_instance()->get_graphics_queue());

        m_color_images.push_back(image);
        m_color_images_memory.push_back(memory);
        m_color_images_view.push_back(image_view);
        m_color_samplers.push_back(sampler);
        m_color_attachments_texture.push_back(texture::construct(guid, image, image_view, sampler, width, height));
    }

    void vk_render_pass_descriptor::create_depth_attachment(const std::string& guid, ui32 width, ui32 height)
    {
        const ui64 key = (static_cast<ui64>(width) << 32) | height;
        const auto iterator = m_depth_attachments_pool.find(key);
        if (iterator != m_depth_attachments_pool.end())
        {
            m_depth_attachment = iterator->second.lock();
            if (m_depth_attachment)
            {
                return;
            }
        }
        m_depth_attachment = std::make_shared<depth_attachment_data>();

        VkFormat depth_format;
        VkBool32 is_depth_format_supported = vk_device::get_supported_depth_format(&depth_format);
        assert(is_depth_format_supported);

        const auto device = vk_device::get_instance()->get_logical_device();
        VkImageCreateInfo image_create_info = {};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = depth_format;
        image_create_info.extent = { width, height, 1 };
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkResult result = vkCreateImage(device, &image_create_info, nullptr, &m_depth_attachment->m_image);
        assert(result == VK_SUCCESS);

        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(device, m_depth_attachment->m_image, &memory_requirements);
        VkMemoryAllocateInfo memory_allocate_info = {};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = vk_device::get_instance()->get_memory_type(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        result = vkAllocateMemory(device, &memory_allocate_info, nullptr, &m_depth_attachment->m_memory);
        assert(result == VK_SUCCESS);
        result = vkBindImageMemory(device, m_depth_attachment->m_image, m_depth_attachment->m_memory, 0);
        assert(result == VK_SUCCESS);

        VkImageViewCreateInfo image_view_create_info = {};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = m_depth_attachment->m_image;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = depth_format;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT || depth_format == VK_FORMAT_D24_UNORM_S8_UINT || depth_format == VK_FORMAT_D16_UNORM_S8_UINT)
        {
            image_view_create_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.layerCount = 1;
        result = vkCreateImageView(device, &image_view_create_info, nullptr, &m_depth_attachment->m_image_view);
        assert(result == VK_SUCCESS);

        VkSamplerCreateInfo sampler_create_info = {};
        sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_create_info.magFilter = VK_FILTER_NEAREST;
        sampler_create_info.minFilter = VK_FILTER_NEAREST;
        sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_create_info.maxLod = 1.f;
        result = vkCreateSampler(device, &sampler_create_info, nullptr, &m_depth_attachment->m_sampler);
        assert(result == VK_SUCCESS);

        m_depth_attachment->m_texture = texture::construct(guid, m_depth_attachment->m_image, m_depth_attachment->m_image_view, m_depth_attachment->m_sampler, width, height);
        VkCommandBuffer command_buffer = vk_utils::create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (depth_format == VK_FORMAT_D16_UNORM_S8_UINT || depth_format == VK_FORMAT_D24_UNORM_S8_UINT || depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT)
        {
            aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        vk_device::get_instance()->set_image_layout(command_buffer, m_depth_attachment->m_image, aspect_mask, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        vk_utils::flush_command_buffer(command_buffer, vk_device::get_instance()->get_graphics_queue());
        m_depth_attachments_pool[key] = m_depth_attachment;
    }

    VkFormat vk_render_pass_descriptor::get_pixel_format(ui32 format)
    {
        switch (format)
        {
            case 71:
                return VK_FORMAT_R8G8B8A8_SRGB;
            case 72:
                return VK_FORMAT_R8G8B8A8_SNORM;
			case 115:
				return VK_FORMAT_R16G16B16A16_SFLOAT;
            case 125:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            default:
                return VK_FORMAT_R8G8B8A8_UNORM;
        }
    }

    VkRenderPass vk_render_pass_descriptor::get_current_render_pass()
    {
        return m_current_render_pass;
    }

    ui32 vk_render_pass_descriptor::get_current_color_attachments_count()
    {
        return m_current_color_attachments_count;
    }

    void vk_render_pass_descriptor::bind()
    {
        const auto current_image_index = vk_device::get_instance()->get_current_image_index();
        const auto command_buffer = vk_device::get_instance()->get_draw_cmd_buffer(current_image_index);
        const auto render_pass = m_mode == e_mode_output ? vk_swap_chain::get_instance()->get_render_pass() : m_render_pass;
        const auto frame_buffer = m_mode == e_mode_output ? vk_device::get_instance()->get_frame_buffer(current_image_index) : m_frame_buffer;

        VkClearValue output_clear_values[2] = {};
		output_clear_values[0].color = { { 0.f, 0.f, 0.f, 1.f } };
        output_clear_values[1].depthStencil = { 1.f, 0 };

        VkRenderPassBeginInfo render_pass_begin_info = {};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = render_pass;
        render_pass_begin_info.framebuffer = frame_buffer;
        render_pass_begin_info.renderArea.extent = { m_frame_width, m_frame_height };
        render_pass_begin_info.clearValueCount = m_mode == e_mode_output ? 2 : static_cast<ui32>(m_clear_values.size());
        render_pass_begin_info.pClearValues = m_mode == e_mode_output ? output_clear_values : m_clear_values.data();
        vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
		viewport.y = static_cast<f32>(m_frame_height);
        viewport.width = static_cast<f32>(m_frame_width);
		viewport.height = -static_cast<f32>(m_frame_height);
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.extent = { m_frame_width, m_frame_height };
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        m_current_render_pass = render_pass;
        m_current_color_attachments_count = m_mode == e_mode_output ? 1 : static_cast<ui32>(m_color_attachments_texture.size());
    }

    void vk_render_pass_descriptor::unbind()
    {
        const auto current_image_index = vk_device::get_instance()->get_current_image_index();
        const auto command_buffer = vk_device::get_instance()->get_draw_cmd_buffer(current_image_index);
        vkCmdEndRenderPass(command_buffer);
        m_current_render_pass = VK_NULL_HANDLE;
        m_current_color_attachments_count = 0;
    }

    std::vector<texture_shared_ptr> vk_render_pass_descriptor::get_color_attachments_texture() const
    {
        return m_color_attachments_texture;
    }

    texture_shared_ptr vk_render_pass_descriptor::get_depth_attachment_texture() const
    {
        return m_depth_attachment ? m_depth_attachment->m_texture : nullptr;
    }
}

#endif
