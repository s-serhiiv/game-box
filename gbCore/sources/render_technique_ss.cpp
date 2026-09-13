//
//  render_technique_ss.cpp
//  gbCore
//
//  Created by sergey.sergeev on 8/17/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#include "render_technique_ss.h"

#if USED_GRAPHICS_API != NO_GRAPHICS_API

#include "texture.h"
#include "material.h"
#include "mesh_2d.h"
#include "mesh_constructor.h"

#if USED_GRAPHICS_API == METAL_API || USED_GRAPHICS_API == VULKAN_API

#include "render_pass_descriptor.h"

#if USED_GRAPHICS_API == METAL_API

#include "mtl_render_encoder.h"
#include "mtl_buffer.h"

#endif

#endif

namespace gb
{
    render_technique_ss::render_technique_ss(ui32 width, ui32 height, const std::string& name) :
    render_technique_base(width, height, name, 0)
    {
       
    }
    
    render_technique_ss::~render_technique_ss()
    {
        gl::command::delete_frame_buffers(1, &m_frame_buffer);
    }
    
    render_technique_ss_shared_ptr render_technique_ss::construct(const std::shared_ptr<ss_technique_configuration>& configuration,
                                                                  const material_shared_ptr& material)
    {
        render_technique_ss_shared_ptr render_technique = std::make_shared<render_technique_ss>(configuration->get_frame_width(),
                                                                                                configuration->get_frame_height(),
                                                                                                configuration->get_guid());
        render_technique->m_material = material;
        
        ui32 color_attachment_id = 0;
        gl::command::create_textures(1, &color_attachment_id);
        gl::command::bind_texture(gl::constant::texture_2d, color_attachment_id);
        gl::command::texture_parameter_i(gl::constant::texture_2d, gl::constant::texture_min_filter, gl::constant::linear);
        gl::command::texture_parameter_i(gl::constant::texture_2d, gl::constant::texture_mag_filter, gl::constant::linear);
        gl::command::texture_parameter_i(gl::constant::texture_2d, gl::constant::texture_wrap_s, gl::constant::clamp_to_edge);
        gl::command::texture_parameter_i(gl::constant::texture_2d, gl::constant::texture_wrap_t, gl::constant::clamp_to_edge);
        gl::command::texture_image2d(gl::constant::texture_2d, 0, gl::constant::rgba_t,
                                     configuration->get_frame_width(),
                                     configuration->get_frame_height(),
                                     0, gl::constant::rgba_t, gl::constant::ui8_t, NULL);
        
        gl::command::create_frame_buffers(1, &render_technique->m_frame_buffer);
        gl::command::bind_frame_buffer(gl::constant::frame_buffer, render_technique->m_frame_buffer);
        gl::command::attach_frame_buffer_texture2d(gl::constant::frame_buffer, gl::constant::color_attachment_0, gl::constant::texture_2d, color_attachment_id, 0);
        
        assert(gl::command::check_frame_buffer_status(gl::constant::frame_buffer) == gl::constant::frame_buffer_complete);
        
        std::string color_attachment_guid = configuration->get_guid();
        color_attachment_guid.append(".color");
        render_technique->m_color_attachment_texture = texture::construct(color_attachment_guid,
                                                        color_attachment_id,
                                                        configuration->get_frame_width(),
                                                        configuration->get_frame_height());
        render_technique->m_color_attachment_texture->set_wrap_mode(gl::constant::clamp_to_edge);
        
        render_technique->m_quad = mesh_constructor::create_screen_quad();
        
#if USED_GRAPHICS_API == METAL_API || USED_GRAPHICS_API == VULKAN_API
        
        render_technique->m_render_pass_descriptor = render_pass_descriptor::construct_ss_render_pass_descriptor(configuration);
        const auto color_attachments_texture = render_technique->m_render_pass_descriptor->get_color_attachments_texture();
        assert(!color_attachments_texture.empty());
        render_technique->m_color_attachment_texture = color_attachments_texture.at(0);
        
#endif
        
        render_technique->m_skip_frames_count = configuration->get_skip_frames();
        render_technique->m_current_skipped_frames_count = configuration->get_skip_frames();
        
        return render_technique;
    }
    
    std::shared_ptr<texture> render_technique_ss::get_color_attachment_texture() const
    {
        assert(m_color_attachment_texture != nullptr);
        return m_color_attachment_texture;
    }
    
    std::shared_ptr<material> render_technique_ss::get_material() const
    {
        assert(m_material != nullptr);
        return m_material;
    }
    
    void render_technique_ss::bind()
    {
        
#if USED_GRAPHICS_API == METAL_API || USED_GRAPHICS_API == VULKAN_API
        
        m_render_pass_descriptor->bind();
        
#endif
        
        gl::command::bind_frame_buffer(gl::constant::frame_buffer, m_frame_buffer);
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
    
    void render_technique_ss::unbind()
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
    
    void render_technique_ss::draw()
    {
        if(m_material->get_shader()->is_loaded() &&
           m_material->get_shader()->is_commited())
        {
            m_quad->draw();
        }
        
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
    
    ui32 render_technique_ss::get_skip_frames_count() const
    {
        return m_skip_frames_count;
    }
    
    ui32 render_technique_ss::get_current_skipped_frames_count() const
    {
        return m_current_skipped_frames_count;
    }
    
    void render_technique_ss::inc_current_skipped_frames_count()
    {
        m_current_skipped_frames_count++;
        if (m_current_skipped_frames_count > m_skip_frames_count)
        {
            m_current_skipped_frames_count = 0;
        }
    }
}

#endif
