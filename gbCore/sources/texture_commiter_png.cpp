//
//  texture_commiter_png.cpp
//  gbCore
//
//  Created by sergey.sergeev on 8/18/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#include "texture_commiter_png.h"
#include "texture.h"
#include "vk_device.h"
#include "vk_initializers.h"
#include "vk_utils.h"

#if USED_GRAPHICS_API == METAL_API

#include "mtl_texture.h"

#endif

namespace gb
{
    texture_commiter_png::texture_commiter_png(const std::string& guid, const resource_shared_ptr& resource) :
    gb::resource_commiter(guid, resource)
    {
        
    }
    
    texture_commiter_png::~texture_commiter_png()
    {
        
    }
    
    void texture_commiter_png::commit(const resource_transfering_data_shared_ptr& transfering_data)
    {
        m_status = e_commiter_status_in_progress;
        assert(m_resource != nullptr);
        
        ui32 texture_id = 0;
		texture_transfering_data_shared_ptr texture_transfering_data = std::static_pointer_cast<gb::texture_transfering_data>(transfering_data);

        gl::command::create_textures(1, &texture_id);
        gl::command::bind_texture(gl::constant::texture_2d, texture_id);

        gl::command::texture_image2d(gl::constant::texture_2d, 0, texture_transfering_data->m_format,
                           texture_transfering_data->m_width, texture_transfering_data->m_height,
                           0, texture_transfering_data->m_format, gl::constant::ui8_t, (void*)&texture_transfering_data->m_data[0]);
        
#if defined(__USE_MIPMAPS__)
        
        gl::command::generate_mipmap(gl::constant::texture_2d);
        
#endif

		texture_transfering_data->m_mips = std::max(1, static_cast<i32>(texture_transfering_data->m_mips));
		assert(texture_transfering_data->m_mips == 1);

#if USED_GRAPHICS_API == VULKAN_API

		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		VkImageUsageFlags image_usage_flags = VK_IMAGE_USAGE_SAMPLED_BIT;

		VkFormatProperties format_properties;
		vkGetPhysicalDeviceFormatProperties(vk_device::get_instance()->get_physical_device(), format, &format_properties);

		VkMemoryAllocateInfo mem_alloc_info = vk_initializers::memory_allocate_info();
		VkMemoryRequirements mem_requirements;

		VkCommandBuffer copy_command = vk_utils::create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		VkBuffer staging_buffer;
		VkDeviceMemory staging_memory;

		VkBufferCreateInfo buffer_create_info = vk_initializers::buffer_create_info();
		buffer_create_info.size = texture_transfering_data->m_size;
		buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(vk_device::get_instance()->get_logical_device(), &buffer_create_info, nullptr, &staging_buffer);
		assert(result == VK_SUCCESS);

		vkGetBufferMemoryRequirements(vk_device::get_instance()->get_logical_device(), staging_buffer, &mem_requirements);

		mem_alloc_info.allocationSize = mem_requirements.size;
		mem_alloc_info.memoryTypeIndex = vk_device::get_instance()->get_memory_type(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		result = vkAllocateMemory(vk_device::get_instance()->get_logical_device(), &mem_alloc_info, nullptr, &staging_memory);
		assert(result == VK_SUCCESS);
		result = vkBindBufferMemory(vk_device::get_instance()->get_logical_device(), staging_buffer, staging_memory, 0);
		assert(result == VK_SUCCESS);

		ui8* data;
		result = vkMapMemory(vk_device::get_instance()->get_logical_device(), staging_memory, 0, mem_requirements.size, 0, (void **)&data);
		assert(result == VK_SUCCESS);
		memcpy(data, texture_transfering_data->m_data, texture_transfering_data->m_size);
		vkUnmapMemory(vk_device::get_instance()->get_logical_device(), staging_memory);

		std::vector<VkBufferImageCopy> buffer_copy_regions;
		ui32 offset = 0;

		for (ui32 i = 0; i < texture_transfering_data->m_mips; i++)
		{
			VkBufferImageCopy buffer_copy_region = {};
			buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			buffer_copy_region.imageSubresource.mipLevel = i;
			buffer_copy_region.imageSubresource.baseArrayLayer = 0;
			buffer_copy_region.imageSubresource.layerCount = 1;
			buffer_copy_region.imageExtent.width = texture_transfering_data->m_width;
			buffer_copy_region.imageExtent.height = texture_transfering_data->m_height;
			buffer_copy_region.imageExtent.depth = 1;
			buffer_copy_region.bufferOffset = offset;

			buffer_copy_regions.push_back(buffer_copy_region);

			offset += texture_transfering_data->m_size;
		}

		VkImageCreateInfo image_create_info = vk_initializers::image_create_info();
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.format = format;
		image_create_info.mipLevels = texture_transfering_data->m_mips;
		image_create_info.arrayLayers = 1;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_create_info.extent = { texture_transfering_data->m_width, texture_transfering_data->m_height, 1 };
		image_create_info.usage = image_usage_flags;

		if (!(image_create_info.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
		{
			image_create_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		result = vkCreateImage(vk_device::get_instance()->get_logical_device(), &image_create_info, nullptr, &texture_transfering_data->m_image);
		assert(result == VK_SUCCESS);

		vkGetImageMemoryRequirements(vk_device::get_instance()->get_logical_device(), texture_transfering_data->m_image, &mem_requirements);

		mem_alloc_info.allocationSize = mem_requirements.size;

		mem_alloc_info.memoryTypeIndex = vk_device::get_instance()->get_memory_type(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		result = vkAllocateMemory(vk_device::get_instance()->get_logical_device(), &mem_alloc_info, nullptr, &texture_transfering_data->m_image_memory);
		assert(result == VK_SUCCESS);
		result = vkBindImageMemory(vk_device::get_instance()->get_logical_device(), texture_transfering_data->m_image, texture_transfering_data->m_image_memory, 0);
		assert(result == VK_SUCCESS);

		VkImageSubresourceRange subresource_range = {};
		subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource_range.baseMipLevel = 0;
		subresource_range.levelCount = texture_transfering_data->m_mips;
		subresource_range.layerCount = 1;

		vk_device::get_instance()->set_image_layout(copy_command, texture_transfering_data->m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresource_range);
		vkCmdCopyBufferToImage(copy_command, staging_buffer, texture_transfering_data->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<ui32>(buffer_copy_regions.size()), buffer_copy_regions.data());

		VkImageLayout image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vk_device::get_instance()->set_image_layout(copy_command, texture_transfering_data->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image_layout, subresource_range);

		VkQueue copy_queue = vk_device::get_instance()->get_graphics_queue();
		vk_utils::flush_command_buffer(copy_command, copy_queue);

		vkFreeMemory(vk_device::get_instance()->get_logical_device(), staging_memory, nullptr);
		vkDestroyBuffer(vk_device::get_instance()->get_logical_device(), staging_buffer, nullptr);

		VkImageViewCreateInfo image_view_info = {};
		image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_info.image = texture_transfering_data->m_image;
		image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_info.format = format;
		image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_info.subresourceRange.baseMipLevel = 0;
		image_view_info.subresourceRange.levelCount = 1;
		image_view_info.subresourceRange.baseArrayLayer = 0;
		image_view_info.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(vk_device::get_instance()->get_logical_device(), &image_view_info, nullptr, &texture_transfering_data->m_image_view));

		VkSamplerCreateInfo sampler_info = {};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.magFilter = VK_FILTER_LINEAR;
		sampler_info.minFilter = VK_FILTER_LINEAR;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		sampler_info.anisotropyEnable = VK_TRUE;
		sampler_info.maxAnisotropy = 16;
		sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		sampler_info.unnormalizedCoordinates = VK_FALSE;
		sampler_info.compareEnable = VK_FALSE;
		sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.mipLodBias = 0.0f;
		sampler_info.minLod = 0.0f;
		sampler_info.maxLod = 0.0f;

		VK_CHECK(vkCreateSampler(vk_device::get_instance()->get_logical_device(), &sampler_info, nullptr, &texture_transfering_data->m_sampler));
		texture_transfering_data->m_is_image_owner = true;

#elif USED_GRAPHICS_API == METAL_API
        
        texture_transfering_data->m_mtl_texture_id = std::make_shared<mtl_texture>(texture_transfering_data->m_width,
                                                                                   texture_transfering_data->m_height,
                                                                                   (void*)&texture_transfering_data->m_data[0]);
        
#endif

        m_status = e_commiter_status_success;
        
        texture_transfering_data->m_texture_id = texture_id;
        resource_commiter::on_transfering_data_commited(texture_transfering_data);
    }
}
