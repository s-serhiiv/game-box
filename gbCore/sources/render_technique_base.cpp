//
//  render_technique_base.cpp
//  gbCore
//
//  Created by sergey.sergeev on 8/17/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#include "render_technique_base.h"

#if USED_GRAPHICS_API == METAL_API || USED_GRAPHICS_API == VULKAN_API

#include "render_pass_descriptor.h"

#endif

#if USED_GRAPHICS_API != NO_GRAPHICS_API

namespace gb
{
    render_technique_base::render_technique_base(ui32 width, ui32 height, const std::string& name, ui32 order) :
	m_frame_buffer(0),
    m_frame_width(width),
    m_frame_height(height),
    m_name(name),
    m_order(order),
    m_clear_color(glm::vec4(0.f, 0.f, 0.f, 1.f))
    {

#if USED_GRAPHICS_API == VULKAN_API

		m_vk_viewport = {};
		m_vk_viewport.x = 0.0f;
		m_vk_viewport.y = 0.0f;
		m_vk_viewport.width = static_cast<float>(width);
		m_vk_viewport.height = static_cast<float>(height);
		m_vk_viewport.minDepth = 0.0f;
		m_vk_viewport.maxDepth = 1.0f;

		m_vk_scissor = {};
		m_vk_scissor.offset = { 0, 0 };
		m_vk_scissor.extent = {width, height};

		m_vk_viewport_state = {};
		m_vk_viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		m_vk_viewport_state.viewportCount = 1;
		m_vk_viewport_state.pViewports = &m_vk_viewport;
		m_vk_viewport_state.scissorCount = 1;
		m_vk_viewport_state.pScissors = &m_vk_scissor;

#endif
        
        m_uniforms[ces_render_technique_uniforms_component::e_shader_uniform_type_vertex] = nullptr;
        m_uniforms[ces_render_technique_uniforms_component::e_shader_uniform_type_fragment] = nullptr;
    }
    
    render_technique_base::~render_technique_base(void)
    {
        
    }
    
    ui32 render_technique_base::get_order(void) const
    {
        return m_order;
    }
    
    ui32 render_technique_base::get_frame_width(void) const
    {
        assert(m_frame_width != 0);
        return m_frame_width;
    }
    
    ui32 render_technique_base::get_frame_height(void) const
    {
        assert(m_frame_height != 0);
        return m_frame_height;
    }
    
    std::string render_technique_base::get_name(void) const
    {
        return m_name;
    }
    
    void render_technique_base::set_clear_color(const glm::vec4& color)
    {
        m_clear_color = color;
    }
    
    void render_technique_base::set_uniforms(const std::shared_ptr<ces_render_technique_uniforms_component::shader_uniforms>& uniforms)
    {
        m_uniforms[uniforms->get_type()] = uniforms;
    }
    
    std::vector<texture_shared_ptr> render_technique_base::get_color_attachments_texture()
    {
        return m_color_attachments_texture;
    }
}

#endif
