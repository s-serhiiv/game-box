//
//  mesh.cpp
//  gbCore
//
//  Created by sergey.sergeev on 8/13/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#include "mesh_2d.h"
#include "resource_status.h"
#include "vk_device.h"
#include "shader.h"

namespace gb
{
    mesh_2d::mesh_2d(const vbo_shared_ptr& vbo, const ibo_shared_ptr& ibo, ui32 mode) : resource(e_resource_type_mesh_2d, ""),
    m_vbo(vbo),
    m_ibo(ibo),
    m_mode(mode)
    {
        
    }
    
    mesh_2d::mesh_2d(e_resource_type type, const std::string& guid) : resource(type, guid),
    m_vbo(nullptr),
    m_ibo(nullptr),
    m_mode(gl::constant::triangles)
    {
        
    }
    
    mesh_2d::~mesh_2d()
    {
        
    }
    
    vbo_shared_ptr mesh_2d::get_vbo() const
    {
        return m_vbo;
    }
    
    ibo_shared_ptr mesh_2d::get_ibo() const
    {
        return m_ibo;
    }
    
    ui32 mesh_2d::get_id() const
    {
        return m_vbo->get_id();
    }
    
    void mesh_2d::on_transfering_data_serialized(const std::shared_ptr<resource_transfering_data>& data)
    {
        assert(false);
    }
    
    void mesh_2d::on_transfering_data_commited(const std::shared_ptr<resource_transfering_data>& data)
    {
        assert(false);
    }
    
    void mesh_2d::bind(const std::string& attributes_guid, const std::array<i32, e_shader_attribute_max>& attributes)
    {

#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API

        assert(attributes_guid.length() != 0);
        std::shared_ptr<vao> vao_state = m_vao_states[attributes_guid];
        if(!vao_state)
        {
            assert(m_vbo->is_using_batch() == m_ibo->is_using_batch());
            vao_state = std::make_shared<vao>(m_vbo->is_using_batch());
            vao::bind(vao_state);
            m_vbo->bind(attributes);
            m_ibo->bind();
            m_vao_states[attributes_guid] = vao_state;
        }
        else
        {
            vao::bind(vao_state);
        }

#elif USED_GRAPHICS_API == VULKAN_API

		m_vbo->bind(attributes);
		m_ibo->bind();

#endif

    }
    
    void mesh_2d::draw() const
    {

#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API

        gl::command::draw_elements(m_mode, m_ibo->get_used_size(), gl::constant::ui16_t, NULL);
        
#elif USED_GRAPHICS_API == VULKAN_API

		ui32 current_image_index = vk_device::get_instance()->get_current_image_index();
		VkCommandBuffer draw_cmd_buffer = vk_device::get_instance()->get_draw_cmd_buffer(current_image_index);
		shader::bind_vulkan_descriptor_set();
		vkCmdDrawIndexed(draw_cmd_buffer, m_ibo->get_used_size(), 1, 0, 0, 0);

#endif

    }
    
    void mesh_2d::draw(ui32 indices) const
    {
        
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
        
        gl::command::draw_elements(m_mode, indices, gl::constant::ui16_t, NULL);
        
#elif USED_GRAPHICS_API == VULKAN_API

		ui32 current_image_index = vk_device::get_instance()->get_current_image_index();
		VkCommandBuffer draw_cmd_buffer = vk_device::get_instance()->get_draw_cmd_buffer(current_image_index);
		shader::bind_vulkan_descriptor_set();
		vkCmdDrawIndexed(draw_cmd_buffer, indices, 1, 0, 0, 0);

#endif

    }
    
    void mesh_2d::unbind(const std::string& attributes_guid, const std::array<i32, e_shader_attribute_max>& attributes)
    {
        vao::unbind();
    }
    
    bool mesh_2d::intersect(const vbo_shared_ptr& vbo_01, const ibo_shared_ptr& ibo_01, const glm::mat4& mat_m_01, bool use_mat_m_01,
                            const vbo_shared_ptr& vbo_02, const ibo_shared_ptr& ibo_02, const glm::mat4& mat_m_02, bool use_mat_m_02,
                            std::vector<glm::triangle>* out_triangles_01, std::vector<glm::triangle>* out_triangles_02)
    {
        std::vector<glm::triangle> triangles_01;
        std::vector<glm::triangle> triangles_02;
        
        i32 indices_01_count = ibo_01->get_used_size();
        i32 indices_02_count = ibo_02->get_used_size();
        
        if(indices_01_count % 3 == 0 && indices_02_count % 3 == 0)
        {
            vbo::vertex_attribute_P *vertices_01 = vbo_01->lock<vbo::vertex_attribute_P>();
            ui16 *indices_01 = ibo_01->lock();
            triangles_01.resize(indices_01_count / 3);
            
            for(i32 i = 0, j = 0; i < indices_01_count; i += 3, ++j)
            {
                if(use_mat_m_01)
                {
                    glm::transform_to_vec2_out(vertices_01[indices_01[i]].m_position, mat_m_01, &triangles_01[j].points[0]);
                    glm::transform_to_vec2_out(vertices_01[indices_01[i + 1]].m_position, mat_m_01, &triangles_01[j].points[1]);
                    glm::transform_to_vec2_out(vertices_01[indices_01[i + 2]].m_position, mat_m_01, &triangles_01[j].points[2]);
                }
                else
                {
                    triangles_01[j].points[0].x = vertices_01[indices_01[i]].m_position.x;
                    triangles_01[j].points[0].y = vertices_01[indices_01[i]].m_position.y;
                    
                    triangles_01[j].points[1].x = vertices_01[indices_01[i + 1]].m_position.x;
                    triangles_01[j].points[1].y = vertices_01[indices_01[i + 1]].m_position.y;
                    
                    triangles_01[j].points[2].x = vertices_01[indices_01[i + 2]].m_position.x;
                    triangles_01[j].points[2].y = vertices_01[indices_01[i + 2]].m_position.y;
                }
                
                triangles_01[j].segments[0].point_01 = triangles_01[j].points[0];
                triangles_01[j].segments[0].point_02 = triangles_01[j].points[1];
                
                triangles_01[j].segments[1].point_01 = triangles_01[j].points[1];
                triangles_01[j].segments[1].point_02 = triangles_01[j].points[2];
                
                triangles_01[j].segments[2].point_01 = triangles_01[j].points[2];
                triangles_01[j].segments[2].point_02 = triangles_01[j].points[0];
            }
            
            vbo::vertex_attribute_P *vertices_02 = vbo_02->lock<vbo::vertex_attribute_P>();
            ui16 *indices_02 = ibo_02->lock();
            triangles_02.resize(indices_02_count / 3);
            
            for(i32 i = 0, j = 0; i < indices_02_count; i += 3, ++j)
            {
                if(use_mat_m_02)
                {
                    glm::transform_to_vec2_out(vertices_02[indices_02[i]].m_position, mat_m_02, &triangles_02[j].points[0]);
                    glm::transform_to_vec2_out(vertices_02[indices_02[i + 1]].m_position, mat_m_02, &triangles_02[j].points[1]);
                    glm::transform_to_vec2_out(vertices_02[indices_02[i + 2]].m_position, mat_m_02, &triangles_02[j].points[2]);
                }
                else
                {
                    triangles_02[j].points[0].x = vertices_02[indices_02[i]].m_position.x;
                    triangles_02[j].points[0].y = vertices_02[indices_02[i]].m_position.y;
                    
                    triangles_02[j].points[1].x = vertices_02[indices_02[i + 1]].m_position.x;
                    triangles_02[j].points[1].y = vertices_02[indices_02[i + 1]].m_position.y;
                    
                    triangles_02[j].points[2].x = vertices_02[indices_02[i + 2]].m_position.x;
                    triangles_02[j].points[2].y = vertices_02[indices_02[i + 2]].m_position.y;
                }
                
                triangles_02[j].segments[0].point_01 = triangles_02[j].points[0];
                triangles_02[j].segments[0].point_02 = triangles_02[j].points[1];
                
                triangles_02[j].segments[1].point_01 = triangles_02[j].points[1];
                triangles_02[j].segments[1].point_02 = triangles_02[j].points[2];
                
                triangles_02[j].segments[2].point_01 = triangles_02[j].points[2];
                triangles_02[j].segments[2].point_02 = triangles_02[j].points[0];
            }
        }
        else
        {
            return false;
        }
        
        for(const auto& triangle_01 : triangles_01)
        {
            for(const auto& triangle_02 : triangles_02)
            {
                for(const auto& segment_01 : triangle_01.segments)
                {
                    for(const auto& segment_02 : triangle_02.segments)
                    {
                        if(glm::segments_intersect(segment_01, segment_02))
                        {
                            if(out_triangles_01)
                            {
                                *out_triangles_01 = std::move(triangles_01);
                            }
                            if(out_triangles_02)
                            {
                                *out_triangles_02 = std::move(triangles_02);
                            }
                            return true;
                        }
                    }
                }
                
                for(const auto& point_01 : triangle_01.points)
                {
                    if(glm::point_in_triangle(point_01, triangle_02))
                    {
                        if(out_triangles_01)
                        {
                            *out_triangles_01 = std::move(triangles_01);
                        }
                        if(out_triangles_02)
                        {
                            *out_triangles_02 = std::move(triangles_02);
                        }
                        return true;
                    }
                }
                
                for(const auto& point_02 : triangle_02.points)
                {
                    if(glm::point_in_triangle(point_02, triangle_01))
                    {
                        if(out_triangles_01)
                        {
                            *out_triangles_01 = std::move(triangles_01);
                        }
                        if(out_triangles_02)
                        {
                            *out_triangles_02 = std::move(triangles_02);
                        }
                        return true;
                    }
                }
            }
        }
        return false;
    }
    
    bool mesh_2d::is_2d() const
    {
        return true;
    }
    
    bool mesh_2d::is_3d() const
    {
        return false;
    }
    
    mesh_2d_shared_ptr mesh_2d::as_2d()
    {
        return std::static_pointer_cast<mesh_2d>(shared_from_this());
    }
    
    mesh_3d_shared_ptr mesh_2d::as_3d()
    {
        return nullptr;
    }
}
