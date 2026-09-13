//
//  texture.h
//  gbCore
//
//  Created by Sergey Sergeev on 8/12/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#pragma once

#include "resource.h"

#if USED_GRAPHICS_API == METAL_API

#include "mtl_texture.h"

#endif

namespace gb
{
    struct texture_transfering_data : public resource_transfering_data
    {
        ui32 m_texture_id;
        ui32 m_width;
        ui32 m_height;
        ui8* m_data;
		ui32 m_size;
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
        
        texture_transfering_data();
        ~texture_transfering_data();
    };
    
    class texture : public resource
    {
    private:
        
        mutable ui32 m_setted_wrap_mode;
        ui32 m_presetted_wrap_mode;
        
        mutable ui32 m_setted_mag_filter;
        ui32 m_presetted_mag_filter;
        
        mutable ui32 m_setted_min_filter;
        ui32 m_presseted_min_filter;
        
        std::shared_ptr<texture_transfering_data> m_data = nullptr;
        
    protected:
        
        virtual void on_transfering_data_serialized(const std::shared_ptr<resource_transfering_data>& data);
        virtual void on_transfering_data_commited(const std::shared_ptr<resource_transfering_data>& data);
        
    public:
        
        texture(const std::string& guid);
        
        static texture_shared_ptr construct(const std::string& guid,
                                            ui32 texture_id,
                                            ui32 width, ui32 height);
        
        static texture_shared_ptr construct(const std::string& guid,
                                            ui32 width,
                                            ui32 height,
                                            ui32 format,
                                            void* pixels);
        
#if USED_GRAPHICS_API == METAL_API
        
        static texture_shared_ptr construct(const std::string& guid,
                                            const mtl_texture_shared_ptr& mtl_texture_id,
                                            ui32 width, ui32 height);
        
#endif

#if USED_GRAPHICS_API == VULKAN_API

        static texture_shared_ptr construct(const std::string& guid,
                                            VkImage image,
                                            VkImageView image_view,
                                            VkSampler sampler,
                                            ui32 width, ui32 height);

        virtual VkImage get_vk_image() const;
        virtual VkImageView get_vk_image_view() const;
        virtual VkSampler get_vk_sampler() const;

#endif
        
        ~texture();
        
        ui32 get_width() const;
        ui32 get_height() const;
        
        const ui8* get_data() const;
        virtual ui32 get_texture_id() const;
        
#if USED_GRAPHICS_API == METAL_API
        
        virtual std::shared_ptr<mtl_texture> get_mtl_texture_id() const;
        
#endif
        
        virtual ui32 get_format() const;
        virtual ui32 get_bpp() const;
        virtual ui32 get_num_mips() const;
        
        virtual bool is_compressed() const;
        
        virtual void set_wrap_mode(ui32 wrap_mode);
        virtual void set_mag_filter(ui32 mag_filter);
        virtual void set_min_filter(ui32 min_filter);
        
        virtual void bind() const;
        virtual void unbind() const;
        
        virtual void update_pixels_data(void* pixels);
    };
};

