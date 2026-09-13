//
//  render_technique_base.h
//  gbCore
//
//  Created by sergey.sergeev on 8/17/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#pragma once

#include "main_headers.h"
#include "ces_render_technique_uniforms_component.h"

#if USED_GRAPHICS_API != NO_GRAPHICS_API

#include "declarations.h"

namespace gb
{
    class render_technique_base
    {
    private:
        
    protected:
        
        ui32 m_frame_width;
        ui32 m_frame_height;
        ui32 m_frame_buffer;
        glm::vec4 m_clear_color;
        std::string m_name;
        ui32 m_order;
        std::vector<texture_shared_ptr> m_color_attachments_texture;

#if USED_GRAPHICS_API == VULKAN_API

		VkViewport m_vk_viewport;
		VkRect2D m_vk_scissor;
		VkPipelineViewportStateCreateInfo m_vk_viewport_state;

#endif
        
#if USED_GRAPHICS_API == METAL_API || USED_GRAPHICS_API == VULKAN_API
      
        render_pass_descriptor_shared_ptr m_render_pass_descriptor = nullptr;
        
#endif
        
        std::array<std::shared_ptr<ces_render_technique_uniforms_component::shader_uniforms>, ces_render_technique_uniforms_component::e_shader_uniform_type_max> m_uniforms;
        
    public:
        
        render_technique_base(ui32 width, ui32 height, const std::string& name, ui32 index);
        virtual ~render_technique_base();
        
        ui32 get_order() const;
        
        ui32 get_frame_width() const;
        ui32 get_frame_height() const;
        
        std::string get_name() const;
        
        void set_clear_color(const glm::vec4& color);
        
        void set_uniforms(const std::shared_ptr<ces_render_technique_uniforms_component::shader_uniforms>& uniforms);
        
        std::vector<texture_shared_ptr> get_color_attachments_texture();
        
        virtual void bind() = 0;
        virtual void unbind() = 0;
        virtual void draw() = 0;
    };
};

#endif
