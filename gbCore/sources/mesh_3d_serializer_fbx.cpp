//
//  mesh_3d_serializer_fbx.cpp
//  gbCore
//
//  Created by serhii.s on 9/14/26.
//

#include "mesh_3d_serializer_fbx.h"
#include "mesh_3d.h"
#include "animation_3d_sequence.h"
#include <ufbx/ufbx.h>
#include <cfloat>

namespace gb
{
    namespace
    {
        struct fbx_vertex_data
        {
            f32 m_position[3];
            f32 m_texcoord[2];
            f32 m_normal[3];
            f32 m_tangent[3];
            f32 m_color[3];
            i32 m_bone_ids[4];
            f32 m_bone_weights[4];
        };

        glm::vec3 to_vec3(const ufbx_vec3& value)
        {
            return glm::vec3(static_cast<f32>(value.x),
                             static_cast<f32>(value.y),
                             static_cast<f32>(value.z));
        }

        glm::quat to_quat(const ufbx_quat& value)
        {
            return glm::quat(static_cast<f32>(value.w),
                             static_cast<f32>(value.x),
                             static_cast<f32>(value.y),
                             static_cast<f32>(value.z));
        }

        std::string get_node_path(const ufbx_node* node, const ufbx_node* root_node)
        {
            if(!node || node == root_node)
            {
                return "";
            }
            return get_node_path(node->parent, root_node).append("/").append(node->name.data, node->name.length);
        }

        void add_bone_node(const ufbx_node* node,
                           const ufbx_node* root_node,
                           std::vector<const ufbx_node*>* bones,
                           std::unordered_map<const ufbx_node*, i32>* bone_indices)
        {
            if(!node || node == root_node || bone_indices->find(node) != bone_indices->end())
            {
                return;
            }
            add_bone_node(node->parent, root_node, bones, bone_indices);
            const auto bone_index = static_cast<i32>(bones->size());
            bones->push_back(node);
            bone_indices->insert(std::make_pair(node, bone_index));
        }

        void get_bones(const ufbx_scene* scene,
                       std::vector<const ufbx_node*>* bones,
                       std::unordered_map<const ufbx_node*, i32>* bone_indices,
                       std::unordered_map<const ufbx_node*, ufbx_matrix>* bind_transformations)
        {
            for(size_t i = 0; i < scene->nodes.count; ++i)
            {
                const auto node = scene->nodes.data[i];
                if(node->bone)
                {
                    add_bone_node(node, scene->root_node, bones, bone_indices);
                }
                if(node->mesh)
                {
                    for(size_t j = 0; j < node->mesh->skin_deformers.count; ++j)
                    {
                        const auto skin = node->mesh->skin_deformers.data[j];
                        for(size_t k = 0; k < skin->clusters.count; ++k)
                        {
                            const auto cluster = skin->clusters.data[k];
                            add_bone_node(cluster->bone_node, scene->root_node, bones, bone_indices);
                            if(bind_transformations->find(cluster->bone_node) == bind_transformations->end())
                            {
                                bind_transformations->insert(std::make_pair(cluster->bone_node, cluster->bind_to_world));
                            }
                        }
                    }
                }
            }
            std::sort(bones->begin(), bones->end(), [scene](const ufbx_node* node_01, const ufbx_node* node_02) {
                return get_node_path(node_01, scene->root_node) < get_node_path(node_02, scene->root_node);
            });
            bone_indices->clear();
            for(size_t i = 0; i < bones->size(); ++i)
            {
                bone_indices->insert(std::make_pair(bones->at(i), static_cast<i32>(i)));
            }
        }

        ufbx_scene* load_scene(const std::string& filename, e_serializer_status* status)
        {
            const auto filestream = resource_serializer::open_stream(filename, status);
            if(*status == e_serializer_status_failure)
            {
                return nullptr;
            }
            filestream->seekg(0, std::ios::end);
            const auto size = filestream->tellg();
            filestream->seekg(0, std::ios::beg);
            if(size <= 0)
            {
                resource_serializer::close_stream(filestream);
                *status = e_serializer_status_failure;
                return nullptr;
            }
            std::vector<ui8> data(static_cast<size_t>(size));
            filestream->read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size));
            resource_serializer::close_stream(filestream);

            ufbx_load_opts options = {};
            options.generate_missing_normals = true;
            options.normalize_normals = true;
            options.normalize_tangents = true;
            options.clean_skin_weights = true;
            options.target_axes.right = UFBX_COORDINATE_AXIS_POSITIVE_X;
            options.target_axes.up = UFBX_COORDINATE_AXIS_POSITIVE_Y;
            options.target_axes.front = UFBX_COORDINATE_AXIS_POSITIVE_Z;
            options.target_unit_meters = 1.0;
            options.space_conversion = UFBX_SPACE_CONVERSION_MODIFY_GEOMETRY;
            options.geometry_transform_handling = UFBX_GEOMETRY_TRANSFORM_HANDLING_MODIFY_GEOMETRY;

            ufbx_error error = {};
            const auto scene = ufbx_load_memory(data.data(), data.size(), &options, &error);
            if(!scene)
            {
                std::cerr<<"unable to load fbx resource: "<<filename<<". "<<std::string(error.description.data, error.description.length)<<std::endl;
                *status = e_serializer_status_failure;
            }
            return scene;
        }
    }

    mesh_3d_serializer_fbx::mesh_3d_serializer_fbx(const std::string& filename,
                                                   const resource_shared_ptr& resource) :
    resource_serializer(filename, resource),
    m_filename(filename)
    {

    }

    mesh_3d_serializer_fbx::~mesh_3d_serializer_fbx()
    {

    }

    void mesh_3d_serializer_fbx::serialize(const resource_transfering_data_shared_ptr& transfering_data)
    {
        assert(m_resource != nullptr);
        m_status = e_serializer_status_in_progress;

        const auto scene = load_scene(m_filename, &m_status);
        if(!scene)
        {
            return;
        }

        std::vector<const ufbx_node*> bones;
        std::unordered_map<const ufbx_node*, i32> bone_indices;
        std::unordered_map<const ufbx_node*, ufbx_matrix> bind_transformations;
        get_bones(scene, &bones, &bone_indices, &bind_transformations);
        if(bones.size() > k_max_bones)
        {
            std::cerr<<"too many bones in fbx resource: "<<m_filename<<std::endl;
            ufbx_free_scene(scene);
            m_status = e_serializer_status_failure;
            return;
        }

        size_t num_indices = 0;
        for(size_t i = 0; i < scene->nodes.count; ++i)
        {
            const auto node = scene->nodes.data[i];
            if(node->mesh && node->visible)
            {
                num_indices += node->mesh->num_triangles * 3;
            }
        }
        if(num_indices == 0)
        {
            std::cerr<<"missing geometry in fbx resource: "<<m_filename<<std::endl;
            ufbx_free_scene(scene);
            m_status = e_serializer_status_failure;
            return;
        }

        std::vector<fbx_vertex_data> source_vertices(num_indices);
        size_t source_vertex_index = 0;
        for(size_t i = 0; i < scene->nodes.count; ++i)
        {
            const auto node = scene->nodes.data[i];
            const auto mesh = node->mesh;
            if(!mesh || !node->visible)
            {
                continue;
            }
            const auto normal_matrix = ufbx_matrix_for_normals(&node->geometry_to_world);
            const auto skin = mesh->skin_deformers.count > 0 ? mesh->skin_deformers.data[0] : nullptr;
            std::vector<ui32> triangle_indices(mesh->max_face_triangles * 3);
            for(size_t face_index = 0; face_index < mesh->faces.count; ++face_index)
            {
                const auto face = mesh->faces.data[face_index];
                const auto num_triangles = ufbx_triangulate_face(triangle_indices.data(), triangle_indices.size(), mesh, face);
                for(ui32 triangle_index = 0; triangle_index < num_triangles; ++triangle_index)
                {
                    const ui32 winding_indices[3] = { 0, 2, 1 };
                    for(ui32 corner_index = 0; corner_index < 3; ++corner_index)
                    {
                        const auto mesh_vertex_index = triangle_indices[triangle_index * 3 + winding_indices[corner_index]];
                        auto& vertex = source_vertices[source_vertex_index++];
                        memset(&vertex, 0, sizeof(fbx_vertex_data));

                        const auto position = ufbx_transform_position(&node->geometry_to_world, ufbx_get_vertex_vec3(&mesh->vertex_position, mesh_vertex_index));
                        const auto normal = ufbx_transform_direction(&normal_matrix, ufbx_get_vertex_vec3(&mesh->vertex_normal, mesh_vertex_index));
                        const auto normalized_normal = glm::normalize(to_vec3(normal));
                        vertex.m_position[0] = static_cast<f32>(position.x);
                        vertex.m_position[1] = static_cast<f32>(position.y);
                        vertex.m_position[2] = static_cast<f32>(position.z);
                        vertex.m_normal[0] = normalized_normal.x;
                        vertex.m_normal[1] = normalized_normal.y;
                        vertex.m_normal[2] = normalized_normal.z;

                        if(mesh->vertex_uv.exists)
                        {
                            const auto texcoord = ufbx_get_vertex_vec2(&mesh->vertex_uv, mesh_vertex_index);
                            vertex.m_texcoord[0] = static_cast<f32>(texcoord.x);
                            vertex.m_texcoord[1] = static_cast<f32>(texcoord.y);
                        }
                        if(mesh->vertex_tangent.exists)
                        {
                            const auto tangent = ufbx_transform_direction(&normal_matrix, ufbx_get_vertex_vec3(&mesh->vertex_tangent, mesh_vertex_index));
                            const auto normalized_tangent = glm::normalize(to_vec3(tangent));
                            vertex.m_tangent[0] = normalized_tangent.x;
                            vertex.m_tangent[1] = normalized_tangent.y;
                            vertex.m_tangent[2] = normalized_tangent.z;
                        }
                        if(mesh->vertex_color.exists)
                        {
                            const auto color = ufbx_get_vertex_vec4(&mesh->vertex_color, mesh_vertex_index);
                            vertex.m_color[0] = static_cast<f32>(color.x);
                            vertex.m_color[1] = static_cast<f32>(color.y);
                            vertex.m_color[2] = static_cast<f32>(color.z);
                        }
                        else
                        {
                            vertex.m_color[0] = 1.f;
                            vertex.m_color[1] = 1.f;
                            vertex.m_color[2] = 1.f;
                        }

                        if(skin)
                        {
                            const auto logical_vertex_index = mesh->vertex_indices.data[mesh_vertex_index];
                            const auto skin_vertex = skin->vertices.data[logical_vertex_index];
                            f32 total_weight = 0.f;
                            const auto num_weights = std::min<size_t>(skin_vertex.num_weights, 4);
                            for(size_t weight_index = 0; weight_index < num_weights; ++weight_index)
                            {
                                const auto skin_weight = skin->weights.data[skin_vertex.weight_begin + weight_index];
                                const auto cluster = skin->clusters.data[skin_weight.cluster_index];
                                const auto bone_it = bone_indices.find(cluster->bone_node);
                                if(bone_it != bone_indices.end())
                                {
                                    vertex.m_bone_ids[weight_index] = bone_it->second;
                                    vertex.m_bone_weights[weight_index] = static_cast<f32>(skin_weight.weight);
                                    total_weight += vertex.m_bone_weights[weight_index];
                                }
                            }
                            if(total_weight > 0.f)
                            {
                                for(size_t weight_index = 0; weight_index < num_weights; ++weight_index)
                                {
                                    vertex.m_bone_weights[weight_index] /= total_weight;
                                }
                            }
                        }
                    }

                    auto& vertex_01 = source_vertices[source_vertex_index - 3];
                    auto& vertex_02 = source_vertices[source_vertex_index - 2];
                    auto& vertex_03 = source_vertices[source_vertex_index - 1];
                    if(glm::length(glm::vec3(vertex_01.m_tangent[0], vertex_01.m_tangent[1], vertex_01.m_tangent[2])) == 0.f)
                    {
                        const glm::vec3 position_01(vertex_01.m_position[0], vertex_01.m_position[1], vertex_01.m_position[2]);
                        const glm::vec3 position_02(vertex_02.m_position[0], vertex_02.m_position[1], vertex_02.m_position[2]);
                        const glm::vec3 position_03(vertex_03.m_position[0], vertex_03.m_position[1], vertex_03.m_position[2]);
                        const glm::vec2 texcoord_01(vertex_01.m_texcoord[0], vertex_01.m_texcoord[1]);
                        const glm::vec2 texcoord_02(vertex_02.m_texcoord[0], vertex_02.m_texcoord[1]);
                        const glm::vec2 texcoord_03(vertex_03.m_texcoord[0], vertex_03.m_texcoord[1]);
                        const auto edge_01 = position_02 - position_01;
                        const auto edge_02 = position_03 - position_01;
                        const auto delta_uv_01 = texcoord_02 - texcoord_01;
                        const auto delta_uv_02 = texcoord_03 - texcoord_01;
                        const auto determinant = delta_uv_01.x * delta_uv_02.y - delta_uv_02.x * delta_uv_01.y;
                        const auto tangent = fabsf(determinant) > .000001f ? glm::normalize((edge_01 * delta_uv_02.y - edge_02 * delta_uv_01.y) / determinant) : glm::vec3(1.f, 0.f, 0.f);
                        for(auto vertex : { &vertex_01, &vertex_02, &vertex_03 })
                        {
                            vertex->m_tangent[0] = tangent.x;
                            vertex->m_tangent[1] = tangent.y;
                            vertex->m_tangent[2] = tangent.z;
                        }
                    }
                }
            }
        }

        if(!bones.empty())
        {
            for(size_t i = 0; i < source_vertex_index; ++i)
            {
                auto& vertex = source_vertices[i];
                if(vertex.m_bone_weights[0] == 0.f &&
                   vertex.m_bone_weights[1] == 0.f &&
                   vertex.m_bone_weights[2] == 0.f &&
                   vertex.m_bone_weights[3] == 0.f)
                {
                    vertex.m_bone_ids[0] = 0;
                    vertex.m_bone_weights[0] = 1.f;
                }
            }
        }

        std::vector<ui32> source_indices(source_vertex_index);
        ufbx_vertex_stream vertex_stream = {};
        vertex_stream.data = source_vertices.data();
        vertex_stream.vertex_count = source_vertex_index;
        vertex_stream.vertex_size = sizeof(fbx_vertex_data);
        ufbx_error error = {};
        const auto num_vertices = ufbx_generate_indices(&vertex_stream, 1, source_indices.data(), source_vertex_index, nullptr, &error);
        if(num_vertices == 0 || num_vertices > UINT16_MAX)
        {
            std::cerr<<"unable to index fbx resource: "<<m_filename<<". "<<std::string(error.description.data, error.description.length)<<std::endl;
            ufbx_free_scene(scene);
            m_status = e_serializer_status_failure;
            return;
        }

        auto vertices = new mesh_3d_vertex_data[num_vertices];
        auto indices = new ui16[source_vertex_index];
        glm::vec3 min_bound(FLT_MAX);
        glm::vec3 max_bound(-FLT_MAX);
        for(ui32 i = 0; i < num_vertices; ++i)
        {
            const auto& source_vertex = source_vertices[i];
            auto& vertex = vertices[i];
            vertex.m_position = glm::vec3(source_vertex.m_position[0], source_vertex.m_position[1], source_vertex.m_position[2]);
            vertex.m_texcoord = glm::vec2(source_vertex.m_texcoord[0], source_vertex.m_texcoord[1]);
            vertex.m_normal = glm::vec3(source_vertex.m_normal[0], source_vertex.m_normal[1], source_vertex.m_normal[2]);
            vertex.m_tangent = glm::vec3(source_vertex.m_tangent[0], source_vertex.m_tangent[1], source_vertex.m_tangent[2]);
            vertex.m_color = glm::vec3(source_vertex.m_color[0], source_vertex.m_color[1], source_vertex.m_color[2]);
            min_bound = glm::min(min_bound, vertex.m_position);
            max_bound = glm::max(max_bound, vertex.m_position);
            for(ui32 weight_index = 0; weight_index < 4; ++weight_index)
            {
                if(source_vertex.m_bone_weights[weight_index] > 0.f)
                {
                    mesh_3d_bone_data bone_data;
                    bone_data.m_id = source_vertex.m_bone_ids[weight_index];
                    bone_data.m_weigth = source_vertex.m_bone_weights[weight_index];
                    vertex.m_bones.push_back(bone_data);
                }
            }
        }
        for(ui32 i = 0; i < source_vertex_index; ++i)
        {
            indices[i] = static_cast<ui16>(source_indices[i]);
        }

        const auto mesh_transfering_data = std::make_shared<mesh_3d_transfering_data>(vertices, indices,
                                                                                      static_cast<ui32>(num_vertices),
                                                                                      static_cast<ui32>(source_vertex_index),
                                                                                      min_bound, max_bound);
        resource_serializer::on_transfering_data_serialized(mesh_transfering_data);

        const auto skeleton_transfering_data = std::make_shared<skeleton_3d_transfering_data>(static_cast<ui32>(bones.size()));
        for(ui32 i = 0; i < bones.size(); ++i)
        {
            const auto bone = bones[i];
            const auto parent_it = bone_indices.find(bone->parent);
            const auto parent_id = parent_it != bone_indices.end() ? parent_it->second : -1;
            skeleton_transfering_data->add_bone(i, parent_id, std::string(bone->name.data, bone->name.length));
        }
        resource_serializer::on_transfering_data_serialized(skeleton_transfering_data);

        std::vector<glm::quat> rotations;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> scales;
        for(const auto bone : bones)
        {
            const auto bind_transformation_it = bind_transformations.find(bone);
            const auto bind_transformation = bind_transformation_it != bind_transformations.end() ?
                                             ufbx_matrix_to_transform(&bind_transformation_it->second) :
                                             ufbx_matrix_to_transform(&bone->node_to_world);
            positions.push_back(to_vec3(bind_transformation.translation));
            rotations.push_back(to_quat(bind_transformation.rotation));
            scales.push_back(to_vec3(bind_transformation.scale));
        }
        std::vector<frame_3d_data_shared_ptr> frames;
        frames.push_back(std::make_shared<frame_3d_data>(rotations, positions, scales));
        const auto bindpose_transfering_data = std::make_shared<sequence_3d_transfering_data>(k_bindpose_animation_name, 30, frames);
        resource_serializer::on_transfering_data_serialized(bindpose_transfering_data);

        ufbx_free_scene(scene);
        m_status = e_serializer_status_success;
    }
}
