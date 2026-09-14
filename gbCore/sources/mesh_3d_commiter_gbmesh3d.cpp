//
//  mesh_3d_commiter_gbmesh3d.cpp
//  gbCore
//
//  Created by serhii serhiiv on 5/12/17.
//  Copyright © 2017 sergey.sergeev. All rights reserved.
//

#include "mesh_3d_commiter_gbmesh3d.h"
#include "mesh_3d.h"
#include "vbo.h"
#include "ibo.h"

namespace gb
{
    mesh_3d_commiter_gbmesh3d::mesh_3d_commiter_gbmesh3d(const std::string& guid, const resource_shared_ptr& resource) :
    resource_commiter(guid, resource)
    {
        
    }
    
    mesh_3d_commiter_gbmesh3d::~mesh_3d_commiter_gbmesh3d()
    {
        
    }
    
    void mesh_3d_commiter_gbmesh3d::commit(const resource_transfering_data_shared_ptr& transfering_data)
    {
        m_status = e_commiter_status_in_progress;
        assert(m_resource != nullptr);
        
        mesh_3d_shared_ptr mesh = std::static_pointer_cast<mesh_3d>(m_resource);
        
        bool is_skinned = false;
        for(ui32 i = 0; i < mesh->get_num_raw_vertices(); ++i)
        {
            is_skinned = is_skinned || !mesh->get_raw_vertices()[i].m_bones.empty();
        }

        vbo_shared_ptr vbo = nullptr;
        if(is_skinned)
        {
            const auto vertex_declaration = std::make_shared<vbo::vertex_declaration_PT4B>(mesh->get_num_raw_vertices());
            vbo = std::make_shared<gb::vbo>(vertex_declaration, gl::constant::static_draw);
            const auto vertices = vbo->lock<vbo::vertex_attribute_PT4B>();
            for(ui32 i = 0; i < mesh->get_num_raw_vertices(); ++i)
            {
                const auto& source_vertex = mesh->get_raw_vertices()[i];
                vertices[i].m_position = source_vertex.m_position;
                vertices[i].m_texcoord = glm::packHalf2x16(source_vertex.m_texcoord);
                vertices[i].m_bone_ids = glm::u8vec4(source_vertex.m_bones.size() >= 1 ? static_cast<ui8>(source_vertex.m_bones[0].m_id) : 0,
                                                     source_vertex.m_bones.size() >= 2 ? static_cast<ui8>(source_vertex.m_bones[1].m_id) : 0,
                                                     source_vertex.m_bones.size() >= 3 ? static_cast<ui8>(source_vertex.m_bones[2].m_id) : 0,
                                                     source_vertex.m_bones.size() >= 4 ? static_cast<ui8>(source_vertex.m_bones[3].m_id) : 0);
                vertices[i].m_bone_weights = glm::u8vec4(source_vertex.m_bones.size() >= 1 ? static_cast<ui8>(source_vertex.m_bones[0].m_weigth * 255.f) : 0,
                                                         source_vertex.m_bones.size() >= 2 ? static_cast<ui8>(source_vertex.m_bones[1].m_weigth * 255.f) : 0,
                                                         source_vertex.m_bones.size() >= 3 ? static_cast<ui8>(source_vertex.m_bones[2].m_weigth * 255.f) : 0,
                                                         source_vertex.m_bones.size() >= 4 ? static_cast<ui8>(source_vertex.m_bones[3].m_weigth * 255.f) : 0);
            }
        }
        else
        {
            const auto vertex_declaration = std::make_shared<vbo::vertex_declaration_PTNTC>(mesh->get_num_raw_vertices());
            vbo = std::make_shared<gb::vbo>(vertex_declaration, gl::constant::static_draw);
            const auto vertices = vbo->lock<vbo::vertex_attribute_PTNTC>();
            for(ui32 i = 0; i < mesh->get_num_raw_vertices(); ++i)
            {
                const auto position = mesh->get_raw_vertices()[i].m_position;
                const auto texcoord = mesh->get_raw_vertices()[i].m_texcoord;
                const auto normal = mesh->get_raw_vertices()[i].m_normal;
                const auto tangent = mesh->get_raw_vertices()[i].m_tangent;
                const auto color = mesh->get_raw_vertices()[i].m_color;
                vertices[i].m_position = position;
                vertices[i].m_texcoord = glm::packHalf2x16(texcoord);
                vertices[i].m_normal = glm::packSnorm4x8(glm::vec4(normal.x, normal.y, normal.z, 0.0));
                vertices[i].m_tangent = glm::packSnorm4x8(glm::vec4(tangent.x, tangent.y, tangent.z, 0.0));
                vertices[i].m_color = glm::u8vec4(glm::clamp(fabsf(color.x), 0.f, 1.f) * 255,
                                                  glm::clamp(fabsf(color.y), 0.f, 1.f) * 255,
                                                  glm::clamp(fabsf(color.z), 0.f, 1.f) * 255,
                                                  255);
            }
        }
        vbo->unlock();
        
        std::shared_ptr<ibo> ibo = std::make_shared<gb::ibo>(mesh->get_num_raw_indices(), gl::constant::static_draw);
        ui16* indices = ibo->lock();
        memcpy(indices, &mesh->get_raw_indices()[0], sizeof(ui16) * mesh->get_num_raw_indices());
        ibo->unlock();
    
        m_status = e_commiter_status_success;
        resource_commiter::on_transfering_data_commited(vbo);
        resource_commiter::on_transfering_data_commited(ibo);
    }
}
