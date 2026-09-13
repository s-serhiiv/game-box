//
//  cubemap_texture_commiter_png.cpp
//  gbCore
//
//  Created by serhii.s on 5/14/19.
//  Copyright © 2019 sergey.sergeev. All rights reserved.
//

#include "cubemap_texture_commiter_png.h"
#include "cubemap_texture.h"

#if USED_GRAPHICS_API == VULKAN_API

#include "vk_device.h"
#include "vk_initializers.h"
#include "vk_utils.h"

#endif

#if USED_GRAPHICS_API == METAL_API

#include "mtl_texture.h"

#endif

namespace gb
{
    cubemap_texture_commiter_png::cubemap_texture_commiter_png(const std::string& guid, const resource_shared_ptr& resource) :
    gb::resource_commiter(guid, resource)
    {
        
    }
    
    void cubemap_texture_commiter_png::commit(const resource_transfering_data_shared_ptr& transfering_data)
    {
        m_status = e_commiter_status_in_progress;
        assert(m_resource != nullptr);
        
        const auto texture_transfering_data = std::static_pointer_cast<gb::cubemap_texture_transfering_data>(transfering_data);
        
#if USED_GRAPHICS_API == VULKAN_API

        const auto device = vk_device::get_instance();
        const VkDevice logical_device = device->get_logical_device();
        const VkDeviceSize face_size = texture_transfering_data->m_size * texture_transfering_data->m_size * 4;
        const VkDeviceSize image_size = face_size * texture_transfering_data->m_data.size();
        assert(texture_transfering_data->m_format == gl::constant::rgba_t);

        VkBuffer staging_buffer = VK_NULL_HANDLE;
        VkDeviceMemory staging_memory = VK_NULL_HANDLE;
        VkResult result = vk_utils::create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                   image_size, &staging_buffer, &staging_memory);
        assert(result == VK_SUCCESS);

        ui8* data = nullptr;
        result = vkMapMemory(logical_device, staging_memory, 0, image_size, 0, reinterpret_cast<void**>(&data));
        assert(result == VK_SUCCESS);
        for (ui32 slice = 0; slice < texture_transfering_data->m_data.size(); ++slice)
        {
            memcpy(data + face_size * slice, texture_transfering_data->m_data[slice], face_size);
        }
        vkUnmapMemory(logical_device, staging_memory);

        VkImageCreateInfo image_create_info = vk_initializers::image_create_info();
        image_create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_create_info.extent = { texture_transfering_data->m_size, texture_transfering_data->m_size, 1 };
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = static_cast<ui32>(texture_transfering_data->m_data.size());
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        result = vkCreateImage(logical_device, &image_create_info, nullptr, &texture_transfering_data->m_image);
        assert(result == VK_SUCCESS);

        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(logical_device, texture_transfering_data->m_image, &memory_requirements);
        VkMemoryAllocateInfo memory_allocate_info = vk_initializers::memory_allocate_info();
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = device->get_memory_type(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        result = vkAllocateMemory(logical_device, &memory_allocate_info, nullptr, &texture_transfering_data->m_image_memory);
        assert(result == VK_SUCCESS);
        result = vkBindImageMemory(logical_device, texture_transfering_data->m_image, texture_transfering_data->m_image_memory, 0);
        assert(result == VK_SUCCESS);

        std::vector<VkBufferImageCopy> buffer_copy_regions;
        for (ui32 slice = 0; slice < texture_transfering_data->m_data.size(); ++slice)
        {
            VkBufferImageCopy buffer_copy_region = {};
            buffer_copy_region.bufferOffset = face_size * slice;
            buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            buffer_copy_region.imageSubresource.baseArrayLayer = slice;
            buffer_copy_region.imageSubresource.layerCount = 1;
            buffer_copy_region.imageExtent = { texture_transfering_data->m_size, texture_transfering_data->m_size, 1 };
            buffer_copy_regions.push_back(buffer_copy_region);
        }

        VkImageSubresourceRange subresource_range = {};
        subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresource_range.levelCount = 1;
        subresource_range.layerCount = static_cast<ui32>(texture_transfering_data->m_data.size());
        VkCommandBuffer copy_command = vk_utils::create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        device->set_image_layout(copy_command, texture_transfering_data->m_image, VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresource_range);
        vkCmdCopyBufferToImage(copy_command, staging_buffer, texture_transfering_data->m_image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<ui32>(buffer_copy_regions.size()), buffer_copy_regions.data());
        device->set_image_layout(copy_command, texture_transfering_data->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresource_range);
        vk_utils::flush_command_buffer(copy_command, device->get_graphics_queue());
        vkDestroyBuffer(logical_device, staging_buffer, nullptr);
        vkFreeMemory(logical_device, staging_memory, nullptr);

        VkImageViewCreateInfo image_view_info = vk_initializers::image_view_create_info();
        image_view_info.image = texture_transfering_data->m_image;
        image_view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        image_view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_view_info.subresourceRange = subresource_range;
        result = vkCreateImageView(logical_device, &image_view_info, nullptr, &texture_transfering_data->m_image_view);
        assert(result == VK_SUCCESS);

        VkSamplerCreateInfo sampler_info = vk_initializers::sampler_create_info();
        sampler_info.magFilter = VK_FILTER_LINEAR;
        sampler_info.minFilter = VK_FILTER_LINEAR;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.maxLod = 1.f;
        result = vkCreateSampler(logical_device, &sampler_info, nullptr, &texture_transfering_data->m_sampler);
        assert(result == VK_SUCCESS);
        texture_transfering_data->m_mips = 1;
        texture_transfering_data->m_is_image_owner = true;

#elif USED_GRAPHICS_API == METAL_API
        
        texture_transfering_data->m_mtl_texture_id = std::make_shared<mtl_texture>(texture_transfering_data->m_size,
                                                                                   texture_transfering_data->m_data);
        
#endif
        
        m_status = e_commiter_status_success;
        
        texture_transfering_data->m_texture_id = 0;
        resource_commiter::on_transfering_data_commited(texture_transfering_data);
    }
}
