//
//  ces_font_component.cpp
//  gbCore
//
//  Created by sergey.sergeev on 1/6/16.
//  Copyright © 2016 sergey.sergeev. All rights reserved.
//

#include "ces_font_component.h"
#include "vbo.h"
#include "ibo.h"
#include "mesh_2d.h"
#include "mesh_3d.h"
#include "texture.h"
#include "common.h"

namespace gb
{
    static const f32 k_font_csf = 3.f;
    static const f32 k_font_invert_csf = 1.f / k_font_csf;
    static const i32 k_font_max_size = 32;
    static const i32 k_font_default_size = 8;
    static const i32 k_font_atlas_size = 1024;
    static const i32 k_max_symbols = 256;
    static const i32 k_max_num_vertices = k_max_symbols * 4;
    static const i32 k_max_num_indices = k_max_symbols * 6;
    std::unordered_map<std::string, std::pair<ftgl::texture_font_t*, texture_shared_ptr>> ces_font_component::m_font_atlases;
    
    ces_font_component::ces_font_component() :
    m_text(""),
    m_font_size(k_font_default_size),
    m_font_name("boogie_nights.ttf"),
    m_font_color(0.f, 0.f, 0.f, 1.f),
    m_min_bound(glm::vec2(0.f)),
    m_max_bound(glm::vec2(0.f)),
    m_is_multiline(false),
    m_max_line_width(0.f)
    {
        i32 num_vertices = k_max_num_vertices;
        const auto vertex_declaration = std::make_shared<vbo::vertex_declaration_PTNTC>(num_vertices);
        const auto vbo = std::make_shared<gb::vbo>(vertex_declaration, gl::constant::dynamic_draw);
        vbo->unlock();
        
        i32 num_indices = k_max_num_indices;
        const auto ibo = std::make_shared<gb::ibo>(num_indices, gl::constant::dynamic_draw);
        ibo->unlock();
        
        m_mesh_2d = std::make_shared<gb::mesh_2d>(vbo, ibo);
        m_mesh_3d = gb::mesh_3d::construct("font.mesh_3d", vbo, ibo);
    }
    
    ces_font_component::~ces_font_component()
    {
        
    }
    
    mesh_2d_shared_ptr ces_font_component::request_mesh_2d()
    {
        const auto font = reconstruct_atlas_texture();
        reconstruct_mesh(font, m_mesh_2d->get_vbo(), m_mesh_2d->get_ibo());
        return m_mesh_2d;
    }
    
    mesh_3d_shared_ptr ces_font_component::request_mesh_3d()
    {
        const auto font = reconstruct_atlas_texture();
        reconstruct_mesh(font, m_mesh_3d->get_vbo(), m_mesh_3d->get_ibo());
        return m_mesh_3d;
    }
    
    void ces_font_component::reconstruct_mesh(ftgl::texture_font_t* font, const vbo_shared_ptr& vbo, const ibo_shared_ptr& ibo)
    {
        vbo::vertex_attribute_PTNTC* vertices = vbo->lock<vbo::vertex_attribute_PTNTC>();
        ui16* indices = ibo->lock();
        
        glm::vec2 position = glm::vec2(0.f);
        
        i32 vertices_offset = 0;
        i32 indices_offset = 0;
        i32 index = 0;
        for(const char& symbol : m_text)
        {
            ftgl::texture_glyph_t *glyph = texture_font_get_glyph(font, &symbol);
            if(glyph != nullptr)
            {
                f32 kerning =  0.f;
                if(index != 0)
                {
                    kerning = texture_glyph_get_kerning(glyph, &m_text.at(index - 1));
                }
                position.x += kerning * k_font_invert_csf;
                
                if(m_is_multiline && (position.x + glyph->offset_x * k_font_invert_csf + glyph->width * k_font_invert_csf) > m_max_line_width)
                {
                    position.x = 0.f;
                    position.y += m_font_size;
                }
                
                f32 s0 = glyph->s0;
                f32 t0 = glyph->t0;
                f32 s1 = glyph->s1;
                f32 t1 = glyph->t1;
                
                f32 x0 = static_cast<f32>(position.x);
                f32 y0 = static_cast<f32>(position.y + static_cast<f32>(glyph->height) * k_font_invert_csf);
                f32 x1 = static_cast<f32>(position.x + static_cast<f32>(glyph->width) * k_font_invert_csf);
                f32 y1 = static_cast<f32>(position.y);
                
                y0 -= glyph->offset_y * k_font_invert_csf;
                y1 -= glyph->offset_y * k_font_invert_csf;
                
                x0 += glyph->offset_x * k_font_invert_csf;
                x1 += glyph->offset_x * k_font_invert_csf;
                
                vertices[vertices_offset++].m_position = glm::vec3(x0, y0 + m_font_size, 0.f);
                vertices[vertices_offset++].m_position = glm::vec3(x0, y1 + m_font_size, 0.f);
                vertices[vertices_offset++].m_position = glm::vec3(x1, y1 + m_font_size, 0.f);
                vertices[vertices_offset++].m_position = glm::vec3(x1, y0 + m_font_size, 0.f);
                
                vertices_offset -= 4;
                
                glm::vec2 texture_correction = glm::vec2(.0f, .0f);
                vertices[vertices_offset++].m_texcoord = glm::packHalf2x16(glm::vec2(s0 + texture_correction.x, t1 - texture_correction.y));
                vertices[vertices_offset++].m_texcoord = glm::packHalf2x16(glm::vec2(s0 + texture_correction.x, t0 + texture_correction.y));
                vertices[vertices_offset++].m_texcoord = glm::packHalf2x16(glm::vec2(s1 - texture_correction.x, t0 + texture_correction.y));
                vertices[vertices_offset++].m_texcoord = glm::packHalf2x16(glm::vec2(s1 - texture_correction.x, t1 - texture_correction.y));
                
                vertices_offset -= 4;
                
                vertices[vertices_offset++].m_color = m_font_color;
                vertices[vertices_offset++].m_color = m_font_color;
                vertices[vertices_offset++].m_color = m_font_color;
                vertices[vertices_offset++].m_color = m_font_color;
                
                indices[indices_offset++] = 0 + vertices_offset - 4;
                indices[indices_offset++] = 1 + vertices_offset - 4;
                indices[indices_offset++] = 2 + vertices_offset - 4;
                indices[indices_offset++] = 0 + vertices_offset - 4;
                indices[indices_offset++] = 2 + vertices_offset - 4;
                indices[indices_offset++] = 3 + vertices_offset - 4;
                
                assert(vertices_offset < k_max_num_vertices);
                assert(indices_offset < k_max_num_indices);
                
                position.x += glyph->advance_x * k_font_invert_csf;
                index++;
            }
        }
        m_max_bound = glm::vec2(m_is_multiline ? m_max_line_width : position.x, position.y + m_font_size);
        
        vbo->unlock(vertices_offset);
        ibo->unlock(indices_offset);
    }
    
    ftgl::texture_font_t* ces_font_component::reconstruct_atlas_texture()
    {
        std::stringstream font_guid;
        font_guid<<m_font_name<<m_font_size<<m_font_mode;
        ftgl::texture_font_t* font = nullptr;
        auto font_it = m_font_atlases.find(font_guid.str());
        if(font_it == m_font_atlases.end())
        {
            ftgl::texture_atlas_t* atlas = ftgl::texture_atlas_new(k_font_atlas_size, k_font_atlas_size, 1);
            font = texture_font_new_from_file(atlas, m_font_size * k_font_csf, bundlepath().append(m_font_name).c_str());
           
            if (m_font_mode == e_font_mode_edge)
            {
                font->rendermode = ftgl::RENDER_OUTLINE_EDGE;
                font->outline_thickness = 1.f;
            }
            else
            {
                font->rendermode = ftgl::RENDER_OUTLINE_POSITIVE;
                font->outline_thickness = .5f;
            }
            texture_font_load_glyphs(font, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!№;%:?*()_+-=.,/|\\\"'@#$^&{}[]");
            
            ui32 format = 0;
            
#if USED_GRAPHICS_API == OPENGL_30_API || USED_GRAPHICS_API == METAL_API || USED_GRAPHICS_API == VULKAN_API
            
            format = gl::constant::red;
            
#elif USED_GRAPHICS_API == OPENGL_20_API
            
            format = gl::constant::luminance;
            
#endif
            
            auto font_texture = gb::texture::construct(font_guid.str(), static_cast<i32>(atlas->width), static_cast<i32>(atlas->height), format, atlas->data);
            font_texture->set_wrap_mode(gl::constant::clamp_to_edge);
            font_texture->set_mag_filter(gl::constant::linear);
            font_texture->set_min_filter(gl::constant::linear);
            m_font_atlases.insert(std::make_pair(font_guid.str(), std::make_pair(font, font_texture)));
            m_texture = font_texture;
        }
        else
        {
            font = std::get<0>(font_it->second);
            m_texture = std::get<1>(font_it->second);
        }
        
        return font;
    }
    
    void ces_font_component::set_font_size(i32 size)
    {
        m_font_size = size;
        m_font_size = std::max(1, std::min(m_font_size, k_font_max_size));
    }
    
	i32 ces_font_component::get_font_size() const
    {
        return m_font_size;
    }
    
    void ces_font_component::set_font_color(const glm::u8vec4& color)
    {
        m_font_color = color;
    }
    
    glm::u8vec4 ces_font_component::get_font_color() const
    {
        return m_font_color;
    }
    
    void ces_font_component::set_text(const std::string& text)
    {
        m_text = text.length() == 0 ? " " : text;
        /*std::transform(m_text.begin(), m_text.end(), m_text.begin(), ::toupper);*/
    }
    
    std::string ces_font_component::get_text() const
    {
        return m_text;
    }
    
    glm::vec2 ces_font_component::get_min_bound() const
    {
        return m_min_bound;
    }
    
    glm::vec2 ces_font_component::get_max_bound() const
    {
        return m_max_bound;
    }
    
    texture_shared_ptr ces_font_component::get_texture() const
    {
        return m_texture;
    }
    
    void ces_font_component::set_multiline(bool value, f32 max_line_width)
    {
        m_is_multiline = value;
        m_max_line_width = max_line_width;
    }
    
    bool ces_font_component::get_is_multiline() const
    {
        return m_is_multiline;
    }
    
    void ces_font_component::set_font_mode(e_font_mode font_mode)
    {
        m_font_mode = font_mode;
    }
    
    ces_font_component::e_font_mode ces_font_component::get_font_mode() const
    {
        return m_font_mode;
    }
    
    void ces_font_component::set_font_name(const std::string& font_name)
    {
        m_font_name = font_name;
    }
}
