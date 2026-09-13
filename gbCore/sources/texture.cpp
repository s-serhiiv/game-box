//
//  texture.cpp
//  gbCore
//
//  Created by Sergey Sergeev on 8/12/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#include "texture.h"
#include "resource_status.h"

#if USED_GRAPHICS_API == VULKAN_API

#include "vk_device.h"
#include "vk_initializers.h"
#include "vk_utils.h"

#endif

namespace gb
{
    static int textures_count = 0;
    
    texture_transfering_data::texture_transfering_data() :
    m_data(nullptr),
    m_width(0),
    m_height(0),
    m_texture_id(0)
    {
         m_type = e_resource_transfering_data_type_texture;

#if USED_GRAPHICS_API == VULKAN_API

        m_image = VK_NULL_HANDLE;
		m_image_memory = VK_NULL_HANDLE;
        m_image_view = VK_NULL_HANDLE;
        m_sampler = VK_NULL_HANDLE;
		m_is_image_owner = false;

#endif
    }
    
    texture_transfering_data::~texture_transfering_data()
    {
        delete[] m_data;
    }
    
    texture::texture(const std::string& guid) :
    gb::resource(e_resource_type_texture, guid),
    m_data(nullptr),
    m_presetted_wrap_mode(gl::constant::repeat),
	m_presetted_mag_filter(gl::constant::nearest),
	m_presseted_min_filter(gl::constant::nearest),
    m_setted_wrap_mode(0),
    m_setted_mag_filter(0),
    m_setted_min_filter(0)
    {
        textures_count++;
        // std::cout<<"textures count: "<<textures_count<<std::endl;
    }
    
    std::shared_ptr<texture> texture::construct(const std::string& guid,
                                                ui32 texture_id,
                                                ui32 width,
                                                ui32 height)
    {
        std::shared_ptr<gb::texture> texture = std::make_shared<gb::texture>(guid);
        texture->m_data = std::make_shared<texture_transfering_data>();
        texture->m_data->m_texture_id = texture_id;
        texture->m_data->m_width = width;
        texture->m_data->m_height = height;
        texture->m_status |= e_resource_status_loaded;
        texture->m_status |= e_resource_status_commited;
        return texture;
    }
    
    texture_shared_ptr texture::construct(const std::string& guid,
                                          ui32 width,
                                          ui32 height,
                                          ui32 format,
                                          void* pixels)
    {
        std::shared_ptr<gb::texture> texture = std::make_shared<gb::texture>(guid);
        texture->m_data = std::make_shared<texture_transfering_data>();
        
        gl::command::create_textures(1, &texture->m_data->m_texture_id);
        gl::command::bind_texture(gl::constant::texture_2d, texture->m_data->m_texture_id);
        gl::command::texture_image2d(gl::constant::texture_2d, 0, format,  static_cast<i32>(width),  static_cast<i32>(height), 0, format, gl::constant::ui8_t, pixels);
        texture->m_data->m_width = width;
        texture->m_data->m_height = height;
        texture->m_data->m_format = format;
        
#if USED_GRAPHICS_API == METAL_API
        
        texture->m_data->m_mtl_texture_id = std::make_shared<mtl_texture>(width, height, pixels, format);

#elif USED_GRAPHICS_API == VULKAN_API

		const bool is_single_channel = format == gl::constant::red;
		const VkFormat vk_format = is_single_channel ? VK_FORMAT_R8_UNORM : VK_FORMAT_R8G8B8A8_UNORM;
		const VkDeviceSize image_size = width * height * (is_single_channel ? 1 : 4);
		const auto device = vk_device::get_instance();
		const VkDevice logical_device = device->get_logical_device();

		VkBuffer staging_buffer = VK_NULL_HANDLE;
		VkDeviceMemory staging_memory = VK_NULL_HANDLE;
		VkResult result = vk_utils::create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
											VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
											image_size, &staging_buffer, &staging_memory, pixels);
		assert(result == VK_SUCCESS);

		VkImageCreateInfo image_create_info = vk_initializers::image_create_info();
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.format = vk_format;
		image_create_info.extent = { width, height, 1 };
		image_create_info.mipLevels = 1;
		image_create_info.arrayLayers = 1;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		result = vkCreateImage(logical_device, &image_create_info, nullptr, &texture->m_data->m_image);
		assert(result == VK_SUCCESS);

		VkMemoryRequirements memory_requirements;
		vkGetImageMemoryRequirements(logical_device, texture->m_data->m_image, &memory_requirements);
		VkMemoryAllocateInfo memory_allocate_info = vk_initializers::memory_allocate_info();
		memory_allocate_info.allocationSize = memory_requirements.size;
		memory_allocate_info.memoryTypeIndex = device->get_memory_type(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		result = vkAllocateMemory(logical_device, &memory_allocate_info, nullptr, &texture->m_data->m_image_memory);
		assert(result == VK_SUCCESS);
		result = vkBindImageMemory(logical_device, texture->m_data->m_image, texture->m_data->m_image_memory, 0);
		assert(result == VK_SUCCESS);

		VkCommandBuffer copy_command = vk_utils::create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		VkImageSubresourceRange subresource_range = {};
		subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource_range.levelCount = 1;
		subresource_range.layerCount = 1;
		device->set_image_layout(copy_command, texture->m_data->m_image, VK_IMAGE_LAYOUT_UNDEFINED,
								 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresource_range);

		VkBufferImageCopy buffer_copy_region = {};
		buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		buffer_copy_region.imageSubresource.layerCount = 1;
		buffer_copy_region.imageExtent = { width, height, 1 };
		vkCmdCopyBufferToImage(copy_command, staging_buffer, texture->m_data->m_image,
							 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy_region);
		device->set_image_layout(copy_command, texture->m_data->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
								 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresource_range);
		vk_utils::flush_command_buffer(copy_command, device->get_graphics_queue());
		vkDestroyBuffer(logical_device, staging_buffer, nullptr);
		vkFreeMemory(logical_device, staging_memory, nullptr);

		VkImageViewCreateInfo image_view_info = vk_initializers::image_view_create_info();
		image_view_info.image = texture->m_data->m_image;
		image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_info.format = vk_format;
		image_view_info.subresourceRange = subresource_range;
		result = vkCreateImageView(logical_device, &image_view_info, nullptr, &texture->m_data->m_image_view);
		assert(result == VK_SUCCESS);

		VkSamplerCreateInfo sampler_info = vk_initializers::sampler_create_info();
		sampler_info.magFilter = VK_FILTER_LINEAR;
		sampler_info.minFilter = VK_FILTER_LINEAR;
		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.maxLod = 1.f;
		result = vkCreateSampler(logical_device, &sampler_info, nullptr, &texture->m_data->m_sampler);
		assert(result == VK_SUCCESS);
		texture->m_data->m_is_image_owner = true;
        
#endif

        texture->m_status |= e_resource_status_loaded;
        texture->m_status |= e_resource_status_commited;
        
        return texture;
    }

#if USED_GRAPHICS_API == VULKAN_API

    texture_shared_ptr texture::construct(const std::string& guid,
                                          VkImage image,
                                          VkImageView image_view,
                                          VkSampler sampler,
                                          ui32 width, ui32 height)
    {
        std::shared_ptr<gb::texture> texture = std::make_shared<gb::texture>(guid);
        texture->m_data = std::make_shared<texture_transfering_data>();
        texture->m_data->m_image = image;
        texture->m_data->m_image_view = image_view;
        texture->m_data->m_sampler = sampler;
        texture->m_data->m_width = width;
        texture->m_data->m_height = height;
        texture->m_status |= e_resource_status_loaded;
        texture->m_status |= e_resource_status_commited;
        return texture;
    }

#endif
    
#if USED_GRAPHICS_API == METAL_API
    
    texture_shared_ptr texture::construct(const std::string& guid,
                                          const mtl_texture_shared_ptr& mtl_texture_id,
                                          ui32 width, ui32 height)
    {
        std::shared_ptr<gb::texture> texture = std::make_shared<gb::texture>(guid);
        texture->m_data = std::make_shared<texture_transfering_data>();
        texture->m_data->m_texture_id = 0;
        texture->m_data->m_mtl_texture_id = mtl_texture_id;
        texture->m_data->m_width = width;
        texture->m_data->m_height = height;
        texture->m_status |= e_resource_status_loaded;
        texture->m_status |= e_resource_status_commited;
        return texture;
    }
    
#endif

#if USED_GRAPHICS_API == VULKAN_API

    VkImage texture::get_vk_image() const
    {
        return m_data ? m_data->m_image : VK_NULL_HANDLE;
    }

    VkImageView texture::get_vk_image_view() const
    {
        return m_data ? m_data->m_image_view : VK_NULL_HANDLE;
    }

    VkSampler texture::get_vk_sampler() const
    {
        return m_data ? m_data->m_sampler : VK_NULL_HANDLE;
    }

#endif
    
    texture::~texture()
    {
        if (m_data)
        {
            gl::command::delete_textures(1, &m_data->m_texture_id);

#if USED_GRAPHICS_API == VULKAN_API

			if (m_data->m_is_image_owner)
			{
				const VkDevice logical_device = vk_device::get_instance()->get_logical_device();
				vkDestroySampler(logical_device, m_data->m_sampler, nullptr);
				vkDestroyImageView(logical_device, m_data->m_image_view, nullptr);
				vkDestroyImage(logical_device, m_data->m_image, nullptr);
				vkFreeMemory(logical_device, m_data->m_image_memory, nullptr);
			}

#endif
		}
        
        textures_count--;
        // std::cout<<"textures count: "<<textures_count<<std::endl;
    }
    
    void texture::on_transfering_data_serialized(const std::shared_ptr<resource_transfering_data> &data)
    {
        assert(data != nullptr);
        switch(data->get_type())
        {
            case e_resource_transfering_data_type_texture:
            {
                m_data = std::static_pointer_cast<texture_transfering_data>(data);
                m_status |= e_resource_status_loaded;
            }
                break;
            default:
            {
                assert(false);
            }
                break;
        }
    }
    
    void texture::on_transfering_data_commited(const std::shared_ptr<resource_transfering_data> &data)
    {
        assert(data != nullptr);
        switch(data->get_type())
        {
            case e_resource_transfering_data_type_texture:
            {
				auto texture_data = std::static_pointer_cast<texture_transfering_data>(data);
                m_data->m_texture_id = texture_data->m_texture_id;

#if USED_GRAPHICS_API == VULKAN_API

				m_data->m_image = texture_data->m_image;
				m_data->m_image_memory = texture_data->m_image_memory;
				m_data->m_image_view = texture_data->m_image_view;
				m_data->m_sampler = texture_data->m_sampler;
				m_data->m_is_image_owner = texture_data->m_is_image_owner;

#endif
                delete[] m_data->m_data;
                m_data->m_data = nullptr;
                m_status |= e_resource_status_commited;
            }
                break;
            default:
            {
                assert(false);
            }
                break;
        }
    }
    
    ui32 texture::get_width() const
    {
        return resource::is_loaded() ? m_data->m_width : 0;
    }
    
    ui32 texture::get_height() const
    {
        return resource::is_loaded() ? m_data->m_height : 0;
    }
    
    const ui8* texture::get_data() const
    {
        return resource::is_loaded() ? m_data->m_data : nullptr;
    }
    
    ui32 texture::get_texture_id() const
    {
        return m_data->m_texture_id;
    }
    
    ui32 texture::get_format() const
    {
        return resource::is_loaded() ? m_data->m_format : 0;
    }
    
    ui32 texture::get_bpp() const
    {
        return resource::is_loaded() ? m_data->m_bpp : 0;
    }
    
    ui32 texture::get_num_mips() const
    {
        return resource::is_loaded() ? m_data->m_mips : 0;
    }
    
    bool texture::is_compressed() const
    {
        return resource::is_loaded() ? m_data->m_compressed : false;
    }
    
    void texture::set_wrap_mode(ui32 wrap_mode)
    {
        m_presetted_wrap_mode = wrap_mode;
    }
    
    void texture::set_mag_filter(ui32 mag_filter)
    {
        m_presetted_mag_filter = mag_filter;
    }
    
    void texture::set_min_filter(ui32 min_filter)
    {
        m_presseted_min_filter = min_filter;
    }
    
    void texture::bind() const
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            gl::command::bind_texture(gl::constant::texture_2d, m_data->m_texture_id);
            if(m_setted_wrap_mode == 0 || m_presetted_wrap_mode != m_setted_wrap_mode)
            {
                m_setted_wrap_mode = m_presetted_wrap_mode;
                gl::command::texture_parameter_i(gl::constant::texture_2d, gl::constant::texture_wrap_s, m_setted_wrap_mode);
                gl::command::texture_parameter_i(gl::constant::texture_2d, gl::constant::texture_wrap_t, m_setted_wrap_mode);
            }
            if(m_setted_mag_filter == 0 || m_presetted_mag_filter != m_setted_mag_filter)
            {
                m_setted_mag_filter = m_presetted_mag_filter;
                gl::command::texture_parameter_i(gl::constant::texture_2d, gl::constant::texture_mag_filter, m_setted_mag_filter);
            }
            if(m_setted_min_filter == 0 || m_presseted_min_filter != m_setted_min_filter)
            {
                m_setted_min_filter = m_presseted_min_filter;
                gl::command::texture_parameter_i(gl::constant::texture_2d, gl::constant::texture_min_filter, m_setted_min_filter);
            }
        }
    }
    
    void texture::unbind() const
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            gl::command::bind_texture(gl::constant::texture_2d, NULL);
        }
    }
    
#if USED_GRAPHICS_API == METAL_API
    
    std::shared_ptr<mtl_texture> texture::get_mtl_texture_id() const
    {
        return m_data ? m_data->m_mtl_texture_id : nullptr;
    }
    
#endif

    void texture::update_pixels_data(void* pixels)
    {
        
#if USED_GRAPHICS_API == METAL_API
        
        m_data->m_mtl_texture_id->update_pixels_data(m_data->m_width, m_data->m_height, pixels, m_data->m_format);
        
#endif
        
    }
}
