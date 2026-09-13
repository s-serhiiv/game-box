//
//  declarations.h
//  gbCore
//
//  Created by sergey.sergeev on 8/24/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#ifndef declarations_h
#define declarations_h

#define forward_decl(__class__)\
class __class__;\
typedef std::shared_ptr<__class__> __class__##_shared_ptr;\
typedef const std::shared_ptr<__class__>& __class__##_const_shared_ptr;\
typedef std::weak_ptr<__class__> __class__##_weak_ptr;\
struct __class__##_weak_comparator { \
    bool operator() (const __class__##_weak_ptr &lv, const __class__##_weak_ptr &rv)const {\
        return lv.lock() < rv.lock();\
    }\
};

namespace gb
{
    forward_decl(window_impl);
    
    class graphics_context;
    typedef std::shared_ptr<graphics_context> graphics_context_shared_ptr;
    
    class render_pipeline;
    typedef std::shared_ptr<render_pipeline> render_pipeline_shared_ptr;
    
    class input_context;
    typedef std::shared_ptr<input_context> input_context_shared_ptr;
    
    class configuration;
    typedef std::shared_ptr<configuration> configuration_shared_ptr;
    
    class sprite_configuration;
    typedef std::shared_ptr<sprite_configuration> sprite_configuration_shared_ptr;
    
    class resource;
    typedef std::shared_ptr<resource> resource_shared_ptr;
    
    struct resource_transfering_data;
    typedef std::shared_ptr<resource_transfering_data> resource_transfering_data_shared_ptr;
    
    class shader;
    typedef std::shared_ptr<shader> shader_shared_ptr;
    
    struct shader_transfering_data;
    typedef std::shared_ptr<shader_transfering_data> shader_transfering_data_shared_ptr;
    
    struct texture_transfering_data;
    typedef std::shared_ptr<texture_transfering_data> texture_transfering_data_shared_ptr;
    
    class resource_serializer;
    typedef std::shared_ptr<resource_serializer> resource_serializer_shared_ptr;
    
    class resource_commiter;
    typedef std::shared_ptr<resource_commiter> resource_commiter_shared_ptr;
    
    class resource_loading_operation;
    typedef std::shared_ptr<resource_loading_operation> resource_loading_operation_shared_ptr;
    
    class resource_loading_interface;
    typedef std::shared_ptr<resource_loading_interface> resource_loading_interface_shared_ptr;
    
    class shader_loading_operation;
    typedef std::shared_ptr<shader_loading_operation> shader_loading_operation_shared_ptr;
    
    class texture_loading_operation;
    typedef std::shared_ptr<texture_loading_operation> texture_loading_operation_shared_ptr;
    
    class configuration_accessor;
    typedef std::shared_ptr<configuration_accessor> configuration_accessor_shared_ptr;
    
    class material;
    typedef std::shared_ptr<material> material_shared_ptr;

    class ces_entity;
    typedef std::shared_ptr<ces_entity> ces_entity_shared_ptr;
    
    class ces_base_system;
    typedef std::shared_ptr<ces_base_system> ces_base_system_shared_ptr;
    
    class ces_base_component;
    typedef std::shared_ptr<ces_base_component> ces_base_component_shared_ptr;
    
    class ces_geometry_component;
    typedef std::shared_ptr<ces_geometry_component> ces_geometry_component_shared_ptr;
    
    class ces_geometry_quad_component;
    typedef std::shared_ptr<ces_geometry_quad_component> ces_geometry_quad_component_shared_ptr;
    
    class ces_geometry_freeform_component;
    typedef std::shared_ptr<ces_geometry_freeform_component> ces_geometry_freeform_component_shared_ptr;

    class ces_material_component;
    typedef std::shared_ptr<ces_material_component> ces_material_component_shared_ptr;
    
    class ces_light_compoment;
    typedef std::shared_ptr<ces_light_compoment> ces_light_compoment_shared_ptr;
    
    class ces_shadow_component;
    typedef std::shared_ptr<ces_shadow_component> ces_shadow_component_shared_ptr;
    
    class ces_convex_hull_component;
    typedef std::shared_ptr<ces_convex_hull_component> ces_convex_hull_component_shared_ptr;
    
    class ces_bound_touch_component;
    typedef std::shared_ptr<ces_bound_touch_component> ces_bound_touch_component_shared_ptr;
    
    class ces_light_mask_component;
    typedef std::shared_ptr<ces_light_mask_component> ces_light_mask_component_shared_ptr;
    
    class ces_action_component;
    typedef std::shared_ptr<ces_action_component> ces_action_component_shared_ptr;
    
    class game_transition;
    typedef std::shared_ptr<game_transition> game_transition_shared_ptr;
    
    class input_context_listener;
    typedef std::shared_ptr<input_context_listener> input_context_listener_shared_ptr;
    
    class touch_listener;
    typedef std::shared_ptr<touch_listener> touch_listener_shared_ptr;
    
    class collision_manager;
    typedef std::shared_ptr<collision_manager> collision_manager_shared_ptr;
    
    class game_command_i;
    typedef std::shared_ptr<game_command_i> game_command_i_shared_ptr;
    
    class game_commands_container;
    typedef std::shared_ptr<game_commands_container> game_commands_container_shared_ptr;
    
    class thread_operation;
    typedef std::shared_ptr<thread_operation> thread_operation_shared_ptr;
    
    class render_technique_ws;
    typedef std::shared_ptr<render_technique_ws> render_technique_ws_shared_ptr;
    
    class render_target;
    typedef std::shared_ptr<render_target> render_target_shared_ptr;
    
    class batch;
    typedef std::shared_ptr<batch> batch_shared_ptr;
    
    class batching_pipeline;
    typedef std::shared_ptr<batching_pipeline> batching_pipeline_shared_ptr;
    
    class ces_box2d_world_component;
    typedef std::shared_ptr<ces_box2d_world_component> ces_box2d_world_component_shared_ptr;

    class ces_box2d_system;
    typedef std::shared_ptr<ces_box2d_system> ces_box2d_system_shared_ptr;
    
        forward_decl(resource_accessor)
		forward_decl(ces_systems_feeder)
		forward_decl(ces_transformation_component)
		forward_decl(ces_transformation_2d_component)
		forward_decl(ces_transformation_3d_component)
		forward_decl(ces_luminous_component)
		forward_decl(ces_entity)
		forward_decl(ces_box2d_body_component)
		forward_decl(ces_font_component)
		forward_decl(camera_2d)
		forward_decl(camera_3d)
		forward_decl(frustum_3d)
		forward_decl(game_object_2d)
		forward_decl(game_object_3d)
		forward_decl(label)
		forward_decl(light_source_2d)
		forward_decl(light_source_3d)
		forward_decl(scene_graph)
		forward_decl(scene_fabricator)
		forward_decl(sprite)
		forward_decl(heightmap)
		forward_decl(texture)
		forward_decl(shape_3d)
		forward_decl(mesh_2d)
		forward_decl(bone_3d_data)
		forward_decl(bone_3d)
		forward_decl(mesh_3d)
		forward_decl(quad_tree)
		forward_decl(mesh_3d_transfering_data)
		forward_decl(skeleton_3d_transfering_data)
		forward_decl(frame_3d_data)
		forward_decl(sequence_3d_transfering_data)
		forward_decl(animation_3d_sequence)
		forward_decl(vbo)
		forward_decl(ibo)
		forward_decl(vao)
		forward_decl(ces_animation_3d_mixer_component)
		forward_decl(ces_skeleton_3d_component)
		forward_decl(ogl_graveyard_controller)
		forward_decl(vertex_declaration)
		forward_decl(i_binding_listener)
		forward_decl(i_binding_property)
		forward_decl(i_binding_model)
		forward_decl(i_binding_event)
        forward_decl(scene_object_3d_data)
        forward_decl(scene_3d_transfering_data)
        forward_decl(scene_3d)
        forward_decl(animation_3d_state)
        forward_decl(bone_3d_mask_data)
        forward_decl(animation_3d_mask_transfering_data)
        forward_decl(bone_3d_mask)
        forward_decl(animation_3d_mask)
        forward_decl(scene_2d_transfering_data)
        forward_decl(scene_2d)
    forward_decl(ces_bound_touch_2d_component);
    forward_decl(ces_bound_touch_3d_component);
    forward_decl(deferred_light_source_3d);
    forward_decl(mtl_depth_stencil_state);
    forward_decl(mtl_render_pipeline_state);
    forward_decl(mtl_texture);
    forward_decl(mtl_render_encoder);
    forward_decl(mtl_buffer);
    forward_decl(mtl_vertex_descriptor);
    forward_decl(mtl_render_pass_descriptor);
    forward_decl(render_pass_descriptor);
    forward_decl(render_technique_ss);
    forward_decl(particle_emitter);
    forward_decl(deferred_point_light_3d);
    forward_decl(deferred_spot_light_3d);
    forward_decl(trail_3d);
    forward_decl(label_3d);
    forward_decl(fcl_script);
    forward_decl(fcl_script_transfering_data);
    forward_decl(cubemap_texture);
    forward_decl(ces_shader_uniforms_component);
};

#endif
