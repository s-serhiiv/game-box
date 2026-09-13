//
//  cubemap_texture.h
//  gbCore
//
//  Created by serhii.s on 5/14/19.
//  Copyright © 2019 sergey.sergeev. All rights reserved.
//

#pragma once

#include "texture.h"

#if USED_GRAPHICS_API == METAL_API

#include "mtl_texture.h"

#endif

namespace gb
{
    struct cubemap_texture_transfering_data : public resource_transfering_data
    {
        ui32 m_texture_id;
        ui32 m_size;
        std::array<ui8*, 6> m_data;
        ui32 m_format;
        ui32 m_bpp;
        ui32 m_mips;
        bool m_compressed;
        
#if USED_GRAPHICS_API == VULKAN_API
        
        VkImage m_image;
        VkDeviceMemory m_image_memory;
        VkImageView m_image_view;
        VkSampler m_sampler;
        bool m_is_image_owner;
        
#elif USED_GRAPHICS_API == METAL_API
        
        std::shared_ptr<mtl_texture> m_mtl_texture_id = nullptr;
        
#endif
        
        cubemap_texture_transfering_data();
        ~cubemap_texture_transfering_data();
    };
    
    class cubemap_texture : public texture
    {
    private:
        
        std::shared_ptr<cubemap_texture_transfering_data> m_data = nullptr;
        
    protected:

        void on_transfering_data_serialized(const std::shared_ptr<resource_transfering_data>& data) override;
        void on_transfering_data_commited(const std::shared_ptr<resource_transfering_data>& data) override;
        
    public:
        
        cubemap_texture(const std::string& guid);
        
        static cubemap_texture_shared_ptr construct(const std::string& guid,
                                                    ui32 texture_id,
                                                    ui32 size);
        
        static cubemap_texture_shared_ptr construct(const std::string& guid,
                                                    ui32 size,
                                                    ui32 format,
                                                    std::array<ui8*, 6> pixels);
        
#if USED_GRAPHICS_API == METAL_API
        
        static cubemap_texture_shared_ptr construct(const std::string& guid,
                                                    const mtl_texture_shared_ptr& mtl_texture_id,
                                                    ui32 size);
        
#endif
        
        ~cubemap_texture();
        
        ui32 get_size() const;
        
        const ui8* get_data(ui32 slice) const;
        ui32 get_texture_id() const override;
        
#if USED_GRAPHICS_API == METAL_API
        
        std::shared_ptr<mtl_texture> get_mtl_texture_id() const override;
        
#endif

#if USED_GRAPHICS_API == VULKAN_API

        VkImage get_vk_image() const override;
        VkImageView get_vk_image_view() const override;
        VkSampler get_vk_sampler() const override;

#endif
        
        ui32 get_format() const override;
        ui32 get_bpp() const override;
        ui32 get_num_mips() const override;
        
        bool is_compressed() const override;
        
        void set_wrap_mode(ui32 wrap_mode) override;
        void set_mag_filter(ui32 mag_filter) override;
        void set_min_filter(ui32 min_filter) override;
        
        void bind() const override;
        void unbind() const override;
    };
};


