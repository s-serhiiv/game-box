//
//  animation_3d_sequence_serializer_fbx.cpp
//  gbCore
//
//  Created by serhii.s on 9/14/26.
//

#include "animation_3d_sequence_serializer_fbx.h"
#include "animation_3d_sequence.h"
#include "mesh_3d.h"
#include <ufbx/ufbx.h>

namespace gb
{
    namespace
    {
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
                           std::unordered_set<const ufbx_node*>* bone_nodes)
        {
            if(!node || node == root_node || bone_nodes->find(node) != bone_nodes->end())
            {
                return;
            }
            add_bone_node(node->parent, root_node, bones, bone_nodes);
            bones->push_back(node);
            bone_nodes->insert(node);
        }

        std::vector<const ufbx_node*> get_bones(const ufbx_scene* scene)
        {
            std::vector<const ufbx_node*> bones;
            std::unordered_set<const ufbx_node*> bone_nodes;
            for(size_t i = 0; i < scene->nodes.count; ++i)
            {
                const auto node = scene->nodes.data[i];
                if(node->bone)
                {
                    add_bone_node(node, scene->root_node, &bones, &bone_nodes);
                }
                if(node->mesh)
                {
                    for(size_t j = 0; j < node->mesh->skin_deformers.count; ++j)
                    {
                        const auto skin = node->mesh->skin_deformers.data[j];
                        for(size_t k = 0; k < skin->clusters.count; ++k)
                        {
                            add_bone_node(skin->clusters.data[k]->bone_node, scene->root_node, &bones, &bone_nodes);
                        }
                    }
                }
            }
            std::sort(bones.begin(), bones.end(), [scene](const ufbx_node* node_01, const ufbx_node* node_02) {
                return get_node_path(node_01, scene->root_node) < get_node_path(node_02, scene->root_node);
            });
            return bones;
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
            options.target_axes.right = UFBX_COORDINATE_AXIS_POSITIVE_X;
            options.target_axes.up = UFBX_COORDINATE_AXIS_POSITIVE_Y;
            options.target_axes.front = UFBX_COORDINATE_AXIS_POSITIVE_Z;
            options.target_unit_meters = 1.0;
            options.space_conversion = UFBX_SPACE_CONVERSION_ADJUST_TRANSFORMS;
            options.geometry_transform_handling = UFBX_GEOMETRY_TRANSFORM_HANDLING_MODIFY_GEOMETRY;

            ufbx_error error = {};
            const auto scene = ufbx_load_memory(data.data(), data.size(), &options, &error);
            if(!scene)
            {
                std::cerr<<"unable to load fbx animation: "<<filename<<". "<<std::string(error.description.data, error.description.length)<<std::endl;
                *status = e_serializer_status_failure;
            }
            return scene;
        }
    }

    animation_3d_sequence_serializer_fbx::animation_3d_sequence_serializer_fbx(const std::string& filename,
                                                                               const resource_shared_ptr& resource) :
    resource_serializer(filename, resource),
    m_filename(filename)
    {

    }

    animation_3d_sequence_serializer_fbx::~animation_3d_sequence_serializer_fbx()
    {

    }

    void animation_3d_sequence_serializer_fbx::serialize(const resource_transfering_data_shared_ptr& transfering_data)
    {
        assert(m_resource != nullptr);
        m_status = e_serializer_status_in_progress;

        auto filename = m_filename;
        std::string animation_name;
        const auto separator_position = filename.find('#');
        if(separator_position != std::string::npos)
        {
            animation_name = filename.substr(separator_position + 1);
            filename = filename.substr(0, separator_position);
        }

        const auto scene = load_scene(filename, &m_status);
        if(!scene)
        {
            return;
        }
        const auto bones = get_bones(scene);
        if(bones.empty() || bones.size() > k_max_bones)
        {
            std::cerr<<"invalid bones count in fbx animation: "<<filename<<std::endl;
            ufbx_free_scene(scene);
            m_status = e_serializer_status_failure;
            return;
        }

        const ufbx_anim_stack* animation_stack = nullptr;
        if(!animation_name.empty())
        {
            for(size_t i = 0; i < scene->anim_stacks.count; ++i)
            {
                const auto candidate = scene->anim_stacks.data[i];
                if(animation_name == std::string(candidate->name.data, candidate->name.length))
                {
                    animation_stack = candidate;
                    break;
                }
            }
            if(!animation_stack)
            {
                std::cerr<<"missing animation take in fbx resource: "<<m_filename<<std::endl;
                ufbx_free_scene(scene);
                m_status = e_serializer_status_failure;
                return;
            }
        }
        else if(scene->anim_stacks.count > 0)
        {
            animation_stack = scene->anim_stacks.data[0];
            animation_name = std::string(animation_stack->name.data, animation_stack->name.length);
        }

        const auto animation = animation_stack ? animation_stack->anim : scene->anim;
        if(!animation)
        {
            std::cerr<<"missing animation in fbx resource: "<<m_filename<<std::endl;
            ufbx_free_scene(scene);
            m_status = e_serializer_status_failure;
            return;
        }
        const auto time_begin = animation_stack ? animation_stack->time_begin : 0.0;
        const auto time_end = animation_stack ? animation_stack->time_end : 0.0;
        const auto animation_fps = std::max(1, static_cast<i32>(round(scene->settings.frames_per_second > 0.0 ? scene->settings.frames_per_second : 30.0)));
        const auto animation_duration = std::max(0.0, time_end - time_begin);
        const auto num_frames = std::max(1, static_cast<i32>(floor(animation_duration * animation_fps + .5)) + 1);

        std::vector<frame_3d_data_shared_ptr> frames;
        frames.reserve(num_frames);
        for(i32 frame_index = 0; frame_index < num_frames; ++frame_index)
        {
            const auto time = std::min(time_begin + static_cast<f64>(frame_index) / animation_fps, time_end);
            ufbx_error error = {};
            const auto evaluated_scene = ufbx_evaluate_scene(scene, animation, time, nullptr, &error);
            if(!evaluated_scene)
            {
                std::cerr<<"unable to evaluate fbx animation: "<<m_filename<<". "<<std::string(error.description.data, error.description.length)<<std::endl;
                ufbx_free_scene(scene);
                m_status = e_serializer_status_failure;
                return;
            }
            std::vector<glm::quat> rotations;
            std::vector<glm::vec3> positions;
            std::vector<glm::vec3> scales;
            rotations.reserve(bones.size());
            positions.reserve(bones.size());
            scales.reserve(bones.size());
            for(const auto bone : bones)
            {
                const auto evaluated_bone = evaluated_scene->nodes.data[bone->typed_id];
                const auto transform = ufbx_matrix_to_transform(&evaluated_bone->node_to_world);
                positions.push_back(to_vec3(transform.translation));
                rotations.push_back(to_quat(transform.rotation));
                scales.push_back(to_vec3(transform.scale));
            }
            frames.push_back(std::make_shared<frame_3d_data>(rotations, positions, scales));
            ufbx_free_scene(evaluated_scene);
        }

        if(animation_name.empty())
        {
            animation_name = filename;
        }
        const auto sequence_transfering_data = std::make_shared<sequence_3d_transfering_data>(animation_name,
                                                                                              animation_fps,
                                                                                              frames);
        resource_serializer::on_transfering_data_serialized(sequence_transfering_data);
        ufbx_free_scene(scene);
        m_status = e_serializer_status_success;
    }
}
