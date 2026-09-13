//
//  mesh_3d.cpp
//  gbCore
//
//  Created by serhii serhiiv on 5/12/17.
//  Copyright © 2017 sergey.sergeev. All rights reserved.
//

#include "mesh_3d.h"
#include "resource_status.h"
#include "bone_3d.h"
#include "animation_3d_sequence.h"
#include "ibo.h"
#include "vao.h"
#include "vk_device.h"
#include "vk_utils.h"
#include "shader.h"

namespace gb
{
    const ui32 k_max_bones = 64;
    
    mesh_3d_transfering_data::mesh_3d_transfering_data(mesh_3d_vertex_data* vertices, ui16* indices,
                                                       ui32 num_vertices, ui32 num_indices,
                                                       const glm::vec3& min_bound, const glm::vec3& max_bound) :
    m_vertices(vertices),
    m_indices(indices),
    m_num_vertices(num_vertices),
    m_num_indices(num_indices),
    m_min_bound(min_bound),
    m_max_bound(max_bound)
    {
        m_type = e_resource_transfering_data_type_mesh_3d;
    }
    
    mesh_3d_transfering_data::~mesh_3d_transfering_data()
    {
        delete[] m_vertices;
        delete[] m_indices;
    }
    
    const mesh_3d_vertex_data* mesh_3d_transfering_data::get_vertices() const
    {
        return m_vertices;
    }
    
    const ui16* mesh_3d_transfering_data::get_indices() const
    {
        return m_indices;
    }
    
    ui32 mesh_3d_transfering_data::get_num_vertices() const
    {
        return m_num_vertices;
    }
    
    ui32 mesh_3d_transfering_data::get_num_indices() const
    {
        return m_num_indices;
    }
    
    const glm::vec3 mesh_3d_transfering_data::get_min_bound() const
    {
        return m_min_bound;
    }
    
    const glm::vec3 mesh_3d_transfering_data::get_max_bound() const
    {
        return m_max_bound;
    }
    
    skeleton_3d_transfering_data::skeleton_3d_transfering_data(ui32 num_bones) :
    m_num_bones(num_bones)
    {
        m_type = e_resource_transfering_data_type_skeleton_3d;
    }
    
    skeleton_3d_transfering_data::~skeleton_3d_transfering_data()
    {
        m_bones_raw_data.clear();
    }
    
    void skeleton_3d_transfering_data::add_bone(i32 id, i32 parent_id, const std::string& name)
    {
        m_bones_raw_data.push_back(std::make_shared<bone_3d_data>(id, parent_id, name));
    }
    
    ui32 skeleton_3d_transfering_data::get_num_bones() const
    {
        return m_num_bones;
    }
    
    const std::vector<bone_3d_data_shared_ptr>& skeleton_3d_transfering_data::get_bones_raw_data() const
    {
        return m_bones_raw_data;
    }
    
    mesh_3d::mesh_3d(const std::string& guid) : mesh_2d(e_resource_type_mesh_3d, guid),
    m_mesh_data(nullptr),
    m_skeleton_data(nullptr)
    {
        m_vbo = nullptr;
        m_ibo = nullptr;
        m_mode = gl::constant::triangles;
    }
    
    mesh_3d_shared_ptr mesh_3d::construct(const std::string& guid,
                                          const vbo_shared_ptr& vbo,
                                          const ibo_shared_ptr& ibo,
                                          const glm::vec3& max_bound,
                                          const glm::vec3& min_bound)
    {
        assert(vbo != nullptr);
        assert(ibo != nullptr);
        
        mesh_3d_shared_ptr mesh = mesh_3d::construct(guid, vbo, ibo);
        mesh->m_min_bound = min_bound;
        mesh->m_max_bound = max_bound;
        return mesh;
    }
    
    mesh_3d_shared_ptr mesh_3d::construct(const std::string& guid,
                                          const vbo_shared_ptr& vbo,
                                          const ibo_shared_ptr& ibo)
    {
        assert(vbo != nullptr);
        assert(ibo != nullptr);
        
        mesh_3d_shared_ptr mesh = std::make_shared<mesh_3d>(guid);
        mesh->m_vbo = vbo;
        mesh->m_ibo = ibo;
        
        vbo::vertex_attribute_PT4B *vertices = vbo->lock<vbo::vertex_attribute_PT4B>();
        ui16* indices = ibo->lock();
        ui32 indices_count = ibo->get_used_size();
        
        glm::vec3 max_bound = glm::vec3(INT16_MIN);
        glm::vec3 min_bound = glm::vec3(INT16_MAX);
        
        for(ui32 i = 0; i < indices_count; ++i)
        {
            glm::vec3 point = vertices[indices[i]].m_position;
            max_bound = glm::max(point, max_bound);
            min_bound = glm::min(point, min_bound);
        }
        mesh->m_min_bound = std::move(min_bound);
        mesh->m_max_bound = std::move(max_bound);
        
        mesh->m_status |= e_resource_status_loaded;
        mesh->m_status |= e_resource_status_commited;
        return mesh;
    }
    
    mesh_3d::~mesh_3d()
    {
        
    }
    
    void mesh_3d::on_transfering_data_serialized(const std::shared_ptr<resource_transfering_data>& data)
    {
        assert(data != nullptr);
        switch(data->get_type())
        {
            case e_resource_transfering_data_type_mesh_3d:
            {
                m_mesh_data = std::static_pointer_cast<mesh_3d_transfering_data>(data);
                m_min_bound = m_mesh_data->get_min_bound();
                m_max_bound = m_mesh_data->get_max_bound();
                
                m_status |= e_resource_status_loaded;
            }
                break;
                
            case e_resource_transfering_data_type_skeleton_3d:
            {
                m_skeleton_data = std::static_pointer_cast<skeleton_3d_transfering_data>(data);
            }
                break;
                
            case e_resource_transfering_data_type_sequence:
            {
                m_bindpose_data = std::static_pointer_cast<sequence_3d_transfering_data>(data);
            }
                break;
                
            default:
            {
                assert(false);
            }
                break;
        }
    }
    
    void mesh_3d::on_transfering_data_commited(const std::shared_ptr<resource_transfering_data>& data)
    {
        assert(data != nullptr);
        switch(data->get_type())
        {
            case e_resource_transfering_data_type_vbo:
            {
                m_vbo = std::static_pointer_cast<vbo>(data);
            }
                break;
                
            case e_resource_transfering_data_type_ibo:
            {
                m_ibo = std::static_pointer_cast<ibo>(data);
            }
                break;
                
            default:
            {
                assert(false);
            }
                break;
        }
        if(m_vbo != nullptr &&
           m_ibo != nullptr)
        {
            m_mesh_data = nullptr;
            m_status |= e_resource_status_commited;
        }
    }
    
    const mesh_3d_vertex_data* mesh_3d::get_raw_vertices() const
    {
        return resource::is_loaded() ? m_mesh_data->get_vertices() : nullptr;
    }
    
    const ui16* mesh_3d::get_raw_indices() const
    {
        return resource::is_loaded() ? m_mesh_data->get_indices() : nullptr;
    }
    
    ui32 mesh_3d::get_num_raw_vertices() const
    {
        return resource::is_loaded() ? m_mesh_data->get_num_vertices() : 0;
    }
    
    ui32 mesh_3d::get_num_raw_indices() const
    {
        return resource::is_loaded() ? m_mesh_data->get_num_indices() : 0;
    }
    
    vbo_shared_ptr mesh_3d::get_vbo() const
    {
        return resource::is_commited() ? m_vbo : nullptr;
    }
    
    ibo_shared_ptr mesh_3d::get_ibo() const
    {
        return resource::is_commited() ? m_ibo : nullptr;
    }
    
    const glm::vec3 mesh_3d::get_min_bound() const
    {
        return resource::is_loaded() ? m_min_bound : std::move(glm::vec3(0.f));
    }
    
    const glm::vec3 mesh_3d::get_max_bound() const
    {
        return resource::is_loaded() ? m_max_bound : std::move(glm::vec3(0.f));
    }
    
    const skeleton_3d_transfering_data_shared_ptr mesh_3d::get_skeleton_data() const
    {
        return resource::is_loaded() ? m_skeleton_data : nullptr;
    }
    
    const sequence_3d_transfering_data_shared_ptr mesh_3d::get_bindpose_data() const
    {
        return resource::is_loaded() ? m_bindpose_data : nullptr;
    }
    
    void mesh_3d::bind(const std::string& attributes_guid, const std::array<i32, e_shader_attribute_max>& attributes)
    {
        if(resource::is_loaded() && resource::is_commited())
        {

#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API

            assert(attributes_guid.length() != 0);
            std::shared_ptr<vao> vao_state = m_vao_states[attributes_guid];
            if(!vao_state)
            {
                vao_state = std::make_shared<vao>();
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
    }
    
    void mesh_3d::draw() const
    {
        if(resource::is_loaded() && resource::is_commited())
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
    }
    
    void mesh_3d::draw(ui32 indices) const
    {
        if(resource::is_loaded() && resource::is_commited())
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
    }
    
    void mesh_3d::unbind(const std::string& attributes_guid, const std::array<i32, e_shader_attribute_max>& attributes)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            vao::unbind();
        }
    }
    
    bool mesh_3d::is_2d() const
    {
        return false;
    }
    
    bool mesh_3d::is_3d() const
    {
        return true;
    }
    
    mesh_2d_shared_ptr mesh_3d::as_2d()
    {
        return nullptr;
    }
    
    mesh_3d_shared_ptr mesh_3d::as_3d()
    {
        return std::static_pointer_cast<mesh_3d>(shared_from_this());
    }
}
