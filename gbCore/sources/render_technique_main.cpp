//
//  render_technique_main.cpp
//  gbCore
//
//  Created by sergey.sergeev on 8/17/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#include "render_technique_main.h"

#if USED_GRAPHICS_API != NO_GRAPHICS_API

#include "material.h"
#include "mesh_2d.h"
#include "shader.h"
#include "mesh_constructor.h"
#include "vk_device.h"
#include "vk_swap_chain.h"

#if USED_GRAPHICS_API == METAL_API || USED_GRAPHICS_API == VULKAN_API

#include "render_pass_descriptor.h"

#if USED_GRAPHICS_API == METAL_API

#include "mtl_render_encoder.h"
#include "mtl_buffer.h"

#endif

#endif

namespace gb
{
    render_technique_main::render_technique_main(ui32 width, ui32 height, const std::string& name, const material_shared_ptr& material, ui32 frame_buffer, ui32 render_buffer) :
    render_technique_base (width, height, name, 0),
    m_render_buffer(render_buffer),
    m_material(material)
    {
        assert(m_material != nullptr);
        m_frame_buffer = frame_buffer;
        m_quad = mesh_constructor::create_screen_quad();
        
#if USED_GRAPHICS_API == METAL_API || USED_GRAPHICS_API == VULKAN_API
        
        m_render_pass_descriptor = render_pass_descriptor::construct_output_render_pass_descriptor(m_name);
        
#endif
        
    }
    
    render_technique_main::~render_technique_main()
    {
        
    }
    
    void render_technique_main::bind()
    {

#if USED_GRAPHICS_API == METAL_API || USED_GRAPHICS_API == VULKAN_API
        
        m_render_pass_descriptor->bind();
        
#endif
        
        gl::command::bind_frame_buffer(gl::constant::frame_buffer, m_frame_buffer);
        gl::command::bind_render_buffer(gl::constant::render_buffer, m_render_buffer);
        gl::command::viewport(0, 0, m_frame_width, m_frame_height);
        
        gl::command::disable(gl::constant::depth_test);
        material::get_cached_parameters()->set_is_depth_test(false);
        gl::command::depth_mask(gl::constant::no);
        material::get_cached_parameters()->set_is_depth_mask(false);
        gl::command::disable(gl::constant::stencil_test);
        material::get_cached_parameters()->set_is_stencil_test(false);
        
        gl::command::clear_color(m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a);
        gl::command::clear(gl::constant::color_buffer_bit | gl::constant::depth_buffer_bit);
        
        if(m_material->get_shader()->is_loaded() &&
           m_material->get_shader()->is_commited())
        {

#if USED_GRAPHICS_API == VULKAN_API

			m_material->bind(m_quad->get_vbo()->get_vertex_input_state());

#elif USED_GRAPHICS_API == METAL_API
            
            m_material->bind(m_quad->get_vbo()->get_mtl_vertex_descriptor());
            
#else
            
			m_material->bind();

#endif

            m_quad->bind(m_material->get_shader()->get_guid(), m_material->get_shader()->get_attributes());
        }
    }
    
    void render_technique_main::unbind()
    {
        if(m_material->get_shader()->is_loaded() &&
           m_material->get_shader()->is_commited())
        {
            m_quad->unbind(m_material->get_shader()->get_guid(), m_material->get_shader()->get_attributes());
            m_material->unbind();
        }
        
#if USED_GRAPHICS_API == METAL_API || USED_GRAPHICS_API == VULKAN_API
        
        m_render_pass_descriptor->unbind();
        
#endif
        
    }
    
    void render_technique_main::draw()
    {
        if(m_material->get_shader()->is_loaded() &&
           m_material->get_shader()->is_commited())
        {
            m_quad->draw();
            
#if USED_GRAPHICS_API == METAL_API
            
            const auto vbo_mtl_buffer_id = m_quad->get_vbo()->get_mtl_buffer_id();
            const auto ibo_mtl_buffer_id = m_quad->get_ibo()->get_mtl_buffer_id();
            const auto render_encoder = m_material->get_render_encoder();
            render_encoder->set_vertex_buffer(vbo_mtl_buffer_id, 0);
            
            if (m_uniforms[ces_render_technique_uniforms_component::e_shader_uniform_type_vertex])
            {
                void* uniforms_value = m_uniforms[ces_render_technique_uniforms_component::e_shader_uniform_type_vertex]->get_values();
                ui32 uniforms_size = m_uniforms[ces_render_technique_uniforms_component::e_shader_uniform_type_vertex]->get_values_size();
                const auto uniforms_buffer_id = m_material->get_custom_uniform_buffer(uniforms_size);
                uniforms_buffer_id->update(uniforms_value, uniforms_size);
                render_encoder->set_vertex_uniforms(uniforms_buffer_id, 1);
            }

            if (m_uniforms[ces_render_technique_uniforms_component::e_shader_uniform_type_fragment])
            {
                void* uniforms_value = m_uniforms[ces_render_technique_uniforms_component::e_shader_uniform_type_fragment]->get_values();
                ui32 uniforms_size = m_uniforms[ces_render_technique_uniforms_component::e_shader_uniform_type_fragment]->get_values_size();
                const auto uniforms_buffer_id = m_material->get_custom_uniform_buffer(uniforms_size);
                uniforms_buffer_id->update(uniforms_value, uniforms_size);
                render_encoder->set_fragment_uniforms(uniforms_buffer_id, 1);
            }
            
            render_encoder->set_index_buffer(ibo_mtl_buffer_id, m_quad->get_ibo()->get_used_size(), 0);
            render_encoder->draw(m_name);
            
#endif
        }
    }
    
    material_shared_ptr render_technique_main::get_material() const
    {
        return m_material;
    }
}

#endif
