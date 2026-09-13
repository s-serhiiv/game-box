//
//  cubemap_texture.cpp
//  gbCore
//
//  Created by serhii.s on 5/14/19.
//  Copyright © 2019 sergey.sergeev. All rights reserved.
//

#include "cubemap_texture.h"
#include "resource_status.h"

#if USED_GRAPHICS_API == VULKAN_API

#include "vk_device.h"

#endif

namespace gb
{
    cubemap_texture_transfering_data::cubemap_texture_transfering_data() :
    m_size(0),
    m_texture_id(0)
    {
        m_data.fill(nullptr);
        m_type = e_resource_transfering_data_type_cubemap_texture;

#if USED_GRAPHICS_API == VULKAN_API

        m_image = VK_NULL_HANDLE;
        m_image_memory = VK_NULL_HANDLE;
        m_image_view = VK_NULL_HANDLE;
        m_sampler = VK_NULL_HANDLE;
        m_is_image_owner = false;

#endif
    }
    
    cubemap_texture_transfering_data::~cubemap_texture_transfering_data()
    {
        for (ui32 i = 0; i < m_data.size(); i++)
        {
            delete [] m_data[i];
        }
    }
    
    cubemap_texture::cubemap_texture(const std::string& guid) :
    gb::texture(guid)
    {
        m_type = e_resource_type_cubemap_texture;
    }
    
    cubemap_texture_shared_ptr cubemap_texture::construct(const std::string& guid,
                                                          ui32 texture_id,
                                                          ui32 size)
    {
        const auto texture = std::make_shared<gb::cubemap_texture>(guid);
        texture->m_data = std::make_shared<cubemap_texture_transfering_data>();
        texture->m_data->m_texture_id = texture_id;
        texture->m_data->m_size = size;
        texture->m_status |= e_resource_status_loaded;
        texture->m_status |= e_resource_status_commited;
        return texture;
    }
    
    cubemap_texture_shared_ptr cubemap_texture::construct(const std::string& guid,
                                                          ui32 size,
                                                          ui32 format,
                                                          std::array<ui8*, 6> pixels)
    {
        const auto texture = std::make_shared<gb::cubemap_texture>(guid);
        texture->m_data = std::make_shared<cubemap_texture_transfering_data>();
        
        texture->m_data->m_size = size;
        
#if USED_GRAPHICS_API == METAL_API
        
        texture->m_data->m_mtl_texture_id = std::make_shared<mtl_texture>(size, pixels, format);
        
#endif
        
        texture->m_status |= e_resource_status_loaded;
        texture->m_status |= e_resource_status_commited;
        
        return texture;
    }
    
#if USED_GRAPHICS_API == METAL_API
    
    cubemap_texture_shared_ptr cubemap_texture::construct(const std::string& guid,
                                                          const mtl_texture_shared_ptr& mtl_texture_id,
                                                          ui32 size)
    {
        const auto texture = std::make_shared<gb::cubemap_texture>(guid);
        texture->m_data = std::make_shared<cubemap_texture_transfering_data>();
        texture->m_data->m_texture_id = 0;
        texture->m_data->m_mtl_texture_id = mtl_texture_id;
        texture->m_data->m_size = size;
        texture->m_status |= e_resource_status_loaded;
        texture->m_status |= e_resource_status_commited;
        return texture;
    }
    
#endif
    
    cubemap_texture::~cubemap_texture()
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
    
    void cubemap_texture::on_transfering_data_serialized(const std::shared_ptr<resource_transfering_data> &data)
    {
        assert(data != nullptr);
        switch(data->get_type())
        {
            case e_resource_transfering_data_type_cubemap_texture:
            {
                m_data = std::static_pointer_cast<cubemap_texture_transfering_data>(data);
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
    
    void cubemap_texture::on_transfering_data_commited(const std::shared_ptr<resource_transfering_data> &data)
    {
        assert(data != nullptr);
        switch(data->get_type())
        {
            case e_resource_transfering_data_type_cubemap_texture:
            {
                m_data->m_texture_id = std::static_pointer_cast<cubemap_texture_transfering_data>(data)->m_texture_id;

#if USED_GRAPHICS_API == VULKAN_API

                const auto texture_data = std::static_pointer_cast<cubemap_texture_transfering_data>(data);
                m_data->m_image = texture_data->m_image;
                m_data->m_image_memory = texture_data->m_image_memory;
                m_data->m_image_view = texture_data->m_image_view;
                m_data->m_sampler = texture_data->m_sampler;
                m_data->m_is_image_owner = texture_data->m_is_image_owner;

#endif
                for (ui32 i = 0; i < m_data->m_data.size(); i++)
                {
                    delete [] m_data->m_data[i];
                }
                m_data->m_data.fill(nullptr);
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
    
    ui32 cubemap_texture::get_size() const
    {
        return resource::is_loaded() ? m_data->m_size : 0;
    }
    
    
    const ui8* cubemap_texture::get_data(ui32 slice) const
    {
        return resource::is_loaded() ? m_data->m_data.at(slice) : nullptr;
    }
    
    ui32 cubemap_texture::get_texture_id() const
    {
        return m_data->m_texture_id;
    }
    
    ui32 cubemap_texture::get_format() const
    {
        return resource::is_loaded() ? m_data->m_format : 0;
    }
    
    ui32 cubemap_texture::get_bpp() const
    {
        return resource::is_loaded() ? m_data->m_bpp : 0;
    }
    
    ui32 cubemap_texture::get_num_mips() const
    {
        return resource::is_loaded() ? m_data->m_mips : 0;
    }
    
    bool cubemap_texture::is_compressed() const
    {
        return resource::is_loaded() ? m_data->m_compressed : false;
    }
    
    void cubemap_texture::set_wrap_mode(ui32 wrap_mode)
    {

    }
    
    void cubemap_texture::set_mag_filter(ui32 mag_filter)
    {

    }
    
    void cubemap_texture::set_min_filter(ui32 min_filter)
    {

    }
    
    void cubemap_texture::bind() const
    {
        
    }
    
    void cubemap_texture::unbind() const
    {
        
    }
    
#if USED_GRAPHICS_API == METAL_API
    
    std::shared_ptr<mtl_texture> cubemap_texture::get_mtl_texture_id() const
    {
        return m_data ? m_data->m_mtl_texture_id : nullptr;
    }
    
#endif

#if USED_GRAPHICS_API == VULKAN_API

    VkImage cubemap_texture::get_vk_image() const
    {
        return m_data ? m_data->m_image : VK_NULL_HANDLE;
    }

    VkImageView cubemap_texture::get_vk_image_view() const
    {
        return m_data ? m_data->m_image_view : VK_NULL_HANDLE;
    }

    VkSampler cubemap_texture::get_vk_sampler() const
    {
        return m_data ? m_data->m_sampler : VK_NULL_HANDLE;
    }

#endif
}
