//
//  scene_fabricator.cpp
//  gbCore
//
//  Created by sergey.sergeev on 8/26/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#include "scene_fabricator.h"
#include "resource_accessor.h"
#include "mesh_2d.h"
#include "material.h"
#include "mesh_constructor.h"
#include "ces_font_component.h"
#include "texture_configuration.h"
#include "sprite_configuration.h"
#include "label_configuration.h"
#include "font_configuration.h"
#include "glyph_configuration.h"
#include "shape_3d_configuration.h"
#include "animation_3d_sequence_configuration.h"
#include "omni_deferred_light_source_3d_configuration.h"
#include "custom_mesh_deferred_light_source_3d_configuration.h"
#include "particle_emitter_configuration.h"
#include "configuration_accessor.h"
#include "sprite.h"
#include "shape_3d.h"
#include "label.h"
#include "label_3d.h"
#include "light_source_2d.h"
#include "ces_geometry_component.h"
#include "ces_material_extension.h"
#include "ces_animation_3d_mixer_component.h"
#include "ces_animation_3d_mask_component.h"
#include "ces_skeleton_3d_component.h"
#include "ces_geometry_3d_component.h"
#include "mesh_3d.h"
#include "mesh_3d_loading_operation.h"
#include "animation_3d_sequence.h"
#include "ces_animation_3d_system.h"
#include "animation_3d_sequence_loading_operation.h"
#include "deferred_point_light_3d.h"
#include "deferred_spot_light_3d.h"
#include "particle_emitter.h"
#include "ces_particle_emitter_component.h"
#include "trail_3d.h"
#include "ces_trail_component.h"
#include "trail_configuration.h"

namespace gb
{
    scene_fabricator::scene_fabricator()
    {
        
    }
    
    scene_fabricator::~scene_fabricator()
    {
        
    }
    
    void scene_fabricator::set_configuration_accessor(const configuration_accessor_shared_ptr& configuration_accessor)
    {
        m_configuration_accessor = configuration_accessor;
    }
    
    void scene_fabricator::set_resource_accessor(const resource_accessor_shared_ptr& resource_accessor)
    {
        m_resource_accessor = resource_accessor;
    }
    
    const configuration_accessor_shared_ptr scene_fabricator::get_configuration_accessor() const
    {
        return m_configuration_accessor;
    }
    
    const resource_accessor_shared_ptr scene_fabricator::get_resource_accessor() const
    {
        return m_resource_accessor;
    }

#if USED_GRAPHICS_API != NO_GRAPHICS_API
    
    bool scene_fabricator::is_using_batch(const std::vector<std::shared_ptr<configuration>>& configurations)
    {
        for(const auto& iterator : configurations)
        {
            std::shared_ptr<material_configuration> material_configuration = std::static_pointer_cast<gb::material_configuration>(iterator);
            if(!material_configuration->get_is_batching())
            {
                return false;
            }
        }
        return true;
    }
    
    void scene_fabricator::add_materials(const ces_entity_shared_ptr& entity,
                                         const std::vector<std::shared_ptr<configuration>>& configurations)
    {
        for(const auto& iterator : configurations)
        {
            std::shared_ptr<material_configuration> material_configuration =
            std::static_pointer_cast<gb::material_configuration>(iterator);
            
            material_shared_ptr material = material::construct(material_configuration);
            gb::material::set_shader(material, material_configuration, m_resource_accessor, true);
            gb::material::set_textures(material, material_configuration, m_resource_accessor, true);
            ces_material_extension::add_material(entity, material_configuration->get_technique_name(), material);
        }
    }
    
#endif
    
    void scene_fabricator::add_animation_sequences_3d(const ces_entity_shared_ptr& entity,
                                                      const mesh_3d_shared_ptr& mesh,
                                                      const std::vector<std::shared_ptr<configuration>>& configurations)
    {
        auto animation_3d_mixer_compoment = entity->get_component<ces_animation_3d_mixer_component>();
        if (!animation_3d_mixer_compoment)
        {
            animation_3d_mixer_compoment = std::make_shared<ces_animation_3d_mixer_component>();
            entity->add_component(animation_3d_mixer_compoment);
        }
        auto skeleton_3d_component = entity->get_component<ces_skeleton_3d_component>();
        if (!skeleton_3d_component)
        {
            skeleton_3d_component = std::make_shared<ces_skeleton_3d_component>();
            entity->add_component(skeleton_3d_component);
        }
        
        animation_3d_mixer_compoment->setup(mesh->get_skeleton_data(),
                                            mesh->get_bindpose_data());
        skeleton_3d_component->setup(mesh->get_skeleton_data(),
                                    mesh->get_bindpose_data());

        
        ces_animation_3d_system::bind_pose(animation_3d_mixer_compoment, skeleton_3d_component);

        for (const auto& iterator : configurations)
        {
            std::shared_ptr<animation_3d_sequence_configuration> animation_3d_sequence_configuration =
            std::static_pointer_cast<gb::animation_3d_sequence_configuration>(iterator);
            
            auto animation_sequence = m_resource_accessor->get_resource<animation_3d_sequence, animation_3d_sequence_loading_operation>(animation_3d_sequence_configuration->get_animation_filename(), true);
            
            i32 animation_start_index = animation_3d_sequence_configuration->get_start_index();
            i32 animation_end_index = animation_3d_sequence_configuration->get_end_index();
            if (animation_sequence->get_num_frames() > animation_start_index &&
                animation_sequence->get_num_frames() > animation_end_index &&
                animation_start_index <= animation_end_index)
            {
                std::vector<frame_3d_data_shared_ptr> frames;
                for (i32 i = animation_start_index; i <= animation_end_index; ++i)
                {
                    frames.push_back(animation_sequence->get_frame(i));
                }
                std::stringstream string_stream_guid;
                string_stream_guid<<animation_sequence->get_guid()<<"_"<<animation_3d_sequence_configuration->get_start_index()<<"_"<<animation_3d_sequence_configuration->get_end_index();
                
                const auto sequence_data = std::make_shared<sequence_3d_transfering_data>(animation_3d_sequence_configuration->get_animation_name(), animation_sequence->get_animation_fps(), frames);
                animation_sequence = animation_3d_sequence::construct(string_stream_guid.str(), sequence_data);
                
            }
            else
            {
                assert(false && "wrong start or end index in animation");
            }

            animation_3d_mixer_compoment->add_animation_sequence(animation_sequence);
            
            const auto animation_mask_configuration_filename = animation_3d_sequence_configuration->get_animation_mask_filename();
            if (animation_mask_configuration_filename.length() != 0)
            {
                auto animation_3d_mask_compoment = entity->get_component<ces_animation_3d_mask_component>();
                if (!animation_3d_mask_compoment)
                {
                    animation_3d_mask_compoment = std::make_shared<ces_animation_3d_mask_component>();
                    entity->add_component(animation_3d_mask_compoment);
                }
                
                const auto animation_mask_configuration = std::static_pointer_cast<gb::animation_3d_mask_configuration>(m_configuration_accessor->get_animation_3d_mask_configuration(animation_mask_configuration_filename));
                const auto bones_mask_configurations = animation_mask_configuration->get_bone_mask_configurations();
                for (const auto& bone_mask_iterator : bones_mask_configurations)
                {
                    const auto animation_bone_mask_configurations = std::static_pointer_cast<gb::animation_3d_bone_mask_configuration>(bone_mask_iterator);
                    const auto bone_name = animation_bone_mask_configurations->get_bone_name();
                    const auto bone_weight = animation_bone_mask_configurations->get_weight();
                    animation_3d_mask_compoment->add_bone_mask(animation_3d_sequence_configuration->get_animation_name(),
                                                               bone_name, bone_weight);
                }
            }
        }
    }
    
    label_shared_ptr scene_fabricator::create_label_2d(const std::string& filename)
    {
        auto label_configuration =
        std::static_pointer_cast<gb::label_configuration>(m_configuration_accessor->get_label_configuration(filename));
        assert(label_configuration);
        label_shared_ptr label = nullptr;
        if(label_configuration)
        {
            label = gb::ces_entity::construct<gb::label>();
            
#if USED_GRAPHICS_API != NO_GRAPHICS_API
            
            scene_fabricator::add_materials(label, label_configuration->get_materials_configurations());
            
#endif
            
        }
        return label;
    }
    
    label_3d_shared_ptr scene_fabricator::create_label_3d(const std::string& filename)
    {
        auto label_configuration =
        std::static_pointer_cast<gb::label_configuration>(m_configuration_accessor->get_label_configuration(filename));
        assert(label_configuration);
        label_3d_shared_ptr label = nullptr;
        if(label_configuration)
        {
            label = gb::ces_entity::construct<gb::label_3d>();
            
#if USED_GRAPHICS_API != NO_GRAPHICS_API
            
            scene_fabricator::add_materials(label, label_configuration->get_materials_configurations());
            
#endif
        
        }
        return label;
    }
    
    light_source_2d_shared_ptr scene_fabricator::create_light_source_2d(const std::string& filename)
    {
        std::shared_ptr<sprite_configuration> sprite_configuration =
        std::static_pointer_cast<gb::sprite_configuration>(m_configuration_accessor->get_sprite_configuration(filename));
        assert(sprite_configuration);
        light_source_2d_shared_ptr light_source = nullptr;
        if(sprite_configuration)
        {
            light_source = gb::ces_entity::construct<gb::light_source_2d>();
            
#if USED_GRAPHICS_API != NO_GRAPHICS_API
            
            scene_fabricator::add_materials(light_source, sprite_configuration->get_materials_configurations());
            
#endif
            
        }
        return light_source;
    }

    sprite_shared_ptr scene_fabricator::create_sprite(const std::string& filename, const std::string& custom_image_filename)
    {
        std::shared_ptr<sprite_configuration> sprite_configuration =
        std::static_pointer_cast<gb::sprite_configuration>(m_configuration_accessor->get_sprite_configuration(filename));
        assert(sprite_configuration);
        sprite_shared_ptr sprite = nullptr;
        if (sprite_configuration)
        {
            bool is_using_batch = scene_fabricator::is_using_batch(sprite_configuration->get_materials_configurations());
            sprite = gb::ces_entity::construct<gb::sprite>(is_using_batch);
            
#if USED_GRAPHICS_API != NO_GRAPHICS_API
            
            if (custom_image_filename.length() != 0)
            {
                for(const auto& material_configuration_it : sprite_configuration->get_materials_configurations())
                {
                    std::shared_ptr<material_configuration> material_configuration =
                    std::static_pointer_cast<gb::material_configuration>(material_configuration_it);
                    for(const auto& texture_configuration_it : material_configuration->get_textures_configurations())
                    {
                        std::shared_ptr<texture_configuration> texture_configuration =
                        std::static_pointer_cast<gb::texture_configuration>(texture_configuration_it);
                        texture_configuration->set_texture_filename(custom_image_filename);
                    }
                }
            }
            
            scene_fabricator::add_materials(sprite, sprite_configuration->get_materials_configurations());
            
#endif
            
        }
        return sprite;
    }
    
    shape_3d_shared_ptr scene_fabricator::create_shape_3d(const std::string& filename, const std::string& custom_mesh_filename)
    {
        std::shared_ptr<shape_3d_configuration> shape_3d_configuration =
        std::static_pointer_cast<gb::shape_3d_configuration>(m_configuration_accessor->get_shape_3d_configuration(filename));
        assert(shape_3d_configuration);
        shape_3d_shared_ptr shape_3d = nullptr;
        if (shape_3d_configuration)
        {
            shape_3d = gb::ces_entity::construct<gb::shape_3d>();
            
            mesh_3d_shared_ptr mesh = nullptr;
            if (custom_mesh_filename == "plane")
            {
                mesh = mesh_constructor::create_plane_3d();
            }
            else if (custom_mesh_filename == "sphere")
            {
                 mesh = mesh_constructor::create_sphere();
            }
            else
            {
                mesh = m_resource_accessor->get_resource<mesh_3d, mesh_3d_loading_operation>(custom_mesh_filename.length() != 0 ? custom_mesh_filename : shape_3d_configuration->get_mesh_filename(), true);
            }
            
            auto geometry_3d_component = shape_3d->get_component<ces_geometry_3d_component>();
            geometry_3d_component->set_mesh(mesh);
            
#if USED_GRAPHICS_API != NO_GRAPHICS_API
            
            scene_fabricator::add_materials(shape_3d, shape_3d_configuration->get_materials_configurations());
            
#endif
            if (shape_3d_configuration->get_animations_configurations().size() > 0)
            {
                scene_fabricator::add_animation_sequences_3d(shape_3d, mesh, shape_3d_configuration->get_animations_configurations());
            }
        }
        return shape_3d;
    }
    
    deferred_point_light_3d_shared_ptr scene_fabricator::create_deferred_point_light_3d(const std::string& filename)
    {
        const auto configuration =
        std::static_pointer_cast<gb::omni_deferred_light_source_3d_configuration>(m_configuration_accessor->get_omni_deferred_light_source_3d_configuration(filename));
        assert(configuration);
        
        deferred_point_light_3d_shared_ptr light_source = nullptr;
        if (configuration)
        {
            light_source = gb::ces_entity::construct<gb::deferred_point_light_3d>();
            light_source->ray_length = configuration->get_radius();
            light_source->color = glm::vec4(configuration->get_color_r(),
                                            configuration->get_color_g(),
                                            configuration->get_color_b(),
                                            1.f);
            
#if USED_GRAPHICS_API != NO_GRAPHICS_API
            
            scene_fabricator::add_materials(light_source, configuration->get_materials_configurations());
            
#endif
            
        }
        return light_source;
    }
    
    deferred_spot_light_3d_shared_ptr scene_fabricator::create_deferred_spot_light_3d(const std::string& filename)
    {
        const auto configuration =
        std::static_pointer_cast<gb::custom_mesh_deferred_light_source_3d_configuration>(m_configuration_accessor->get_custom_mesh_deferred_light_source_3d_configuration(filename));
        assert(configuration);
        
        deferred_spot_light_3d_shared_ptr light_source = nullptr;
        if(configuration)
        {
            light_source = gb::ces_entity::construct<gb::deferred_spot_light_3d>();
            light_source->color = glm::vec4(configuration->get_color_r(),
                                            configuration->get_color_g(),
                                            configuration->get_color_b(),
                                            1.f);
            
            auto mesh = m_resource_accessor->get_resource<mesh_3d, mesh_3d_loading_operation>(configuration->get_mesh_filename(), true);
            auto geometry_3d_component = light_source->get_component<ces_geometry_3d_component>();
            geometry_3d_component->set_mesh(mesh);
            
#if USED_GRAPHICS_API != NO_GRAPHICS_API
            
            scene_fabricator::add_materials(light_source, configuration->get_materials_configurations());
            
#endif
            
        }
        return light_source;
    }
    
    particle_emitter_shared_ptr scene_fabricator::create_particle_emitter(const std::string& filename)
    {
        std::shared_ptr<particle_emitter_configuration> particle_emitter_configuration =
        std::static_pointer_cast<gb::particle_emitter_configuration>(m_configuration_accessor->get_particle_emitter_configuration(filename));
        assert(particle_emitter_configuration);
        particle_emitter_shared_ptr particle_emitter = nullptr;
        if (particle_emitter_configuration)
        {
            particle_emitter = gb::ces_entity::construct<gb::particle_emitter>();
            const auto particle_emitter_component = particle_emitter->get_component<ces_particle_emitter_component>();
            std::shared_ptr<ces_particle_emitter_component::emitter_settings> settings = std::make_shared<ces_particle_emitter_component::emitter_settings>();
            settings->m_num_particles = particle_emitter_configuration->get_num_particles();
            settings->m_live_time = particle_emitter_configuration->get_live_time();
            settings->m_duration = particle_emitter_configuration->get_duration();
            settings->m_source_size = glm::vec2(particle_emitter_configuration->get_source_size_x(),
                                                particle_emitter_configuration->get_source_size_y());
            settings->m_source_color = glm::u8vec4(particle_emitter_configuration->get_source_color_r(),
                                                   particle_emitter_configuration->get_source_color_g(),
                                                   particle_emitter_configuration->get_source_color_b(),
                                                   particle_emitter_configuration->get_source_color_a());
            
            settings->m_destination_size = glm::vec2(particle_emitter_configuration->get_destination_size_x(),
                                                     particle_emitter_configuration->get_destination_size_y());
            settings->m_destination_color = glm::u8vec4(particle_emitter_configuration->get_destination_color_r(),
                                                        particle_emitter_configuration->get_destination_color_g(),
                                                        particle_emitter_configuration->get_destination_color_b(),
                                                        particle_emitter_configuration->get_destination_color_a());
            
            settings->m_min_horizontal_velocity = particle_emitter_configuration->get_min_horizontal_velocity();
            settings->m_max_horizontal_velocity = particle_emitter_configuration->get_max_horizontal_velocity();
            
            settings->m_min_vertical_velocity = particle_emitter_configuration->get_min_vertical_velocity();
            settings->m_max_vertical_velocity = particle_emitter_configuration->get_max_vertical_velocity();
            
            settings->m_velocity_sensitivity = particle_emitter_configuration->get_velocity_sensitivity();
            
            settings->m_min_emitt_interval = particle_emitter_configuration->get_min_emitt_interval();
            settings->m_max_emitt_interval = particle_emitter_configuration->get_max_emitt_interval();
            
            settings->m_end_velocity = particle_emitter_configuration->get_end_velocity();
            
            settings->m_gravity = glm::vec3(particle_emitter_configuration->get_gravity_x(),
                                            particle_emitter_configuration->get_gravity_y(),
                                            particle_emitter_configuration->get_gravity_z());
            
            particle_emitter_component->set_settings(settings);
            
            auto geometry_3d_component = particle_emitter->get_component<ces_geometry_3d_component>();
            geometry_3d_component->set_mesh(particle_emitter_component->construct_particles_mesh());
            
#if USED_GRAPHICS_API != NO_GRAPHICS_API
            
            scene_fabricator::add_materials(particle_emitter, particle_emitter_configuration->get_materials_configurations());
            
#endif
        }
        return particle_emitter;
    }
    
    trail_3d_shared_ptr scene_fabricator::create_trail_3d(const std::string& filename)
    {
        const auto configuration =
        std::static_pointer_cast<gb::trail_configuration>(m_configuration_accessor->get_trail_configuration(filename));
        assert(configuration);
        
        trail_3d_shared_ptr trail = nullptr;
        if(configuration)
        {
            trail = gb::ces_entity::construct<gb::trail_3d>();
            const auto trail_component = trail->get_component<ces_trail_component>();
            trail_component->set_parameters(configuration->get_segments(), configuration->get_segment_length(), configuration->get_width());
            
            auto geometry_3d_component = trail->get_component<ces_geometry_3d_component>();
            geometry_3d_component->set_mesh(mesh_constructor::create_trail(configuration->get_segments()));
            
#if USED_GRAPHICS_API != NO_GRAPHICS_API
            
            scene_fabricator::add_materials(trail, configuration->get_materials_configurations());
            
#endif
            
        }
        return trail;
    }
}
