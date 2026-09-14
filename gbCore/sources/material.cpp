//
//  material.cpp
//  gbCore
//
//  Created by sergey.sergeev on 8/13/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#include "material.h"

#if USED_GRAPHICS_API != NO_GRAPHICS_API

#include "resource.h"
#include "texture.h"
#include "cubemap_texture.h"
#include "shader_loading_operation.h"
#include "texture_loading_operation.h"
#include "cubemap_texture_loading_operation.h"
#include "blending_configuration.h"
#include "vk_initializers.h"
#include "vk_device.h"
#include "vk_swap_chain.h"
#include "vk_render_pass_descriptor.h"
#include "vk_utils.h"

#if USED_GRAPHICS_API == METAL_API

#include "mtl_render_encoder.h"
#include "mtl_render_pipeline_state.h"
#include "mtl_depth_stencil_state.h"
#include "mtl_buffer.h"
#include "mtl_vertex_descriptor.h"

#endif

namespace gb
    {
    bool blending_parameters::get_is_blending() const
    {
        return m_is_blending;
    }
    
    ui32 blending_parameters::get_blending_function_source() const
    {
        return m_blending_function_source;
    }
    
    ui32 blending_parameters::get_blending_function_destination() const
    {
        return m_blending_function_destination;
    }
    
    ui32 blending_parameters::get_blending_equation() const
    {
        return m_blending_equation;
    }
    
    ui32 blending_parameters::get_attachment_index() const
    {
        return m_attachment_index;
    }
    
    void blending_parameters::set_is_blending(bool value)
    {
        m_is_guid_computed = false;
        m_is_blending = value;
    }
    
    void blending_parameters::set_blending_function_source(ui32 value)
    {
        m_is_guid_computed = false;
        m_blending_function_source = value;
    }
    
    void blending_parameters::set_blending_function_destination(ui32 value)
    {
        m_is_guid_computed = false;
        m_blending_function_destination = value;
    }
    
    void blending_parameters::set_blending_equation(ui32 value)
    {
        m_is_guid_computed = false;
        m_blending_equation = value;
    }
    
    void blending_parameters::set_attachment_index(ui32 value)
    {
        m_is_guid_computed = false;
        m_attachment_index = value;
    }
    
    std::string blending_parameters::get_guid()
    {
        if (!m_is_guid_computed)
        {
            std::stringstream guid_stream;
            guid_stream<<m_is_blending;
            guid_stream<<m_blending_function_source;
            guid_stream<<m_blending_function_destination;
            guid_stream<<m_blending_equation;
            guid_stream<<m_attachment_index;
            m_guid = guid_stream.str();
            m_is_guid_computed = true;
        }
        return m_guid;
    }
    
    material_cached_parameters::material_cached_parameters() :
    m_shader(nullptr)
    {
        std::for_each(m_textures.begin(), m_textures.end(), [](std::shared_ptr<texture>& iterator){
            iterator = nullptr;
        });
        
        m_is_depth_test = true;
        m_is_depth_mask = true;
        
        m_is_culling = false;
        m_culling_mode = gl::constant::back;
        
        m_blending_parameters.resize(1, std::make_shared<blending_parameters>());
        
        m_blending_parameters.at(0)->set_is_blending(false);
        m_blending_parameters.at(0)->set_blending_equation(gl::constant::func_add);
        m_blending_parameters.at(0)->set_blending_function_source(gl::constant::src_alpha);
        m_blending_parameters.at(0)->set_blending_function_destination(gl::constant::one_minus_src_alpha);
        
        gl::command::disable(gl::constant::blend);
        gl::command::blend_function(m_blending_parameters.at(0)->get_blending_function_source(), m_blending_parameters.at(0)->get_blending_function_destination());
        gl::command::blend_equation(m_blending_parameters.at(0)->get_blending_equation());
        
        m_is_stencil_test = false;
        m_stencil_function = gl::constant::always;
        m_stencil_mask_read = 0;
        m_stencil_mask_write = 0;
        m_stencil_ref_value = 255;
        
        m_back_stencil_op_1 = gl::constant::stencil_op_replace;
        m_back_stencil_op_2 = gl::constant::stencil_op_replace;
        m_back_stencil_op_3 = gl::constant::stencil_op_replace;
        
        m_front_stencil_op_1 = gl::constant::stencil_op_replace;
        m_front_stencil_op_2 = gl::constant::stencil_op_replace;
        m_front_stencil_op_3 = gl::constant::stencil_op_replace;
        
        m_is_color_mask_r = true;
        m_is_color_mask_g = true;
        m_is_color_mask_b = true;
        m_is_color_mask_a = true;
        
        m_is_batching = false;
        
        m_z_order = 0;
    }
    
    std::string material_cached_parameters::get_technique_name() const
    {
        return m_technique_name;
    }
    
    void material_cached_parameters::set_technique_name(const std::string& value)
    {
        m_technique_name = value;
    }
    
    bool material_cached_parameters::get_is_culling() const
    {
        return m_is_culling;
    }
    
    ui32 material_cached_parameters::get_culling_mode() const
    {
        return m_culling_mode;
    }
    
    bool material_cached_parameters::get_is_stencil_test() const
    {
        return m_is_stencil_test;
    }
    
    ui32 material_cached_parameters::get_stencil_function() const
    {
        return m_stencil_function;
    }
    
    i32 material_cached_parameters::get_stencil_mask_read() const
    {
        return m_stencil_mask_read;
    }
    
    i32 material_cached_parameters::get_stencil_mask_write() const
    {
        return m_stencil_mask_write;
    }
    
    i32 material_cached_parameters::get_stencil_ref_value() const
    {
        return m_stencil_ref_value;
    }
    
    i32 material_cached_parameters::get_back_stencil_op_1() const
    {
        return m_back_stencil_op_1;
    }
    
    i32 material_cached_parameters::get_back_stencil_op_2() const
    {
        return m_back_stencil_op_2;
    }
    
    i32 material_cached_parameters::get_back_stencil_op_3() const
    {
        return m_back_stencil_op_3;
    }
    
    i32 material_cached_parameters::get_front_stencil_op_1() const
    {
        return m_front_stencil_op_1;
    }
    
    i32 material_cached_parameters::get_front_stencil_op_2() const
    {
        return m_front_stencil_op_2;
    }
    
    i32 material_cached_parameters::get_front_stencil_op_3() const
    {
        return m_front_stencil_op_3;
    }
    
    bool material_cached_parameters::get_is_depth_test() const
    {
        return m_is_depth_test;
    }
    
    bool material_cached_parameters::get_is_depth_mask() const
    {
        return m_is_depth_mask;
    }
    
    bool material_cached_parameters::get_is_color_mask_r() const
    {
        return m_is_color_mask_r;
    }
    
    bool material_cached_parameters::get_is_color_mask_g() const
    {
        return m_is_color_mask_g;
    }
    
    bool material_cached_parameters::get_is_color_mask_b() const
    {
        return m_is_color_mask_b;
    }
    
    bool material_cached_parameters::get_is_color_mask_a() const
    {
        return m_is_color_mask_a;
    }
    
    bool material_cached_parameters::get_is_batching() const
    {
        return m_is_batching;
    }
    
    ui32 material_cached_parameters::get_z_order() const
    {
        return m_z_order;
    }
    
    const std::vector<std::shared_ptr<blending_parameters>>& material_cached_parameters::get_blending_parameters() const
    {
        return m_blending_parameters;
    }
    
    shader_shared_ptr material_cached_parameters::get_shader() const
    {
        return m_shader;
    }
    
    const std::array<texture_shared_ptr, e_shader_sampler_max>& material_cached_parameters::get_textures() const
    {
        return m_textures;
    }
    
    void material_cached_parameters::set_is_culling(bool value)
    {
        m_is_culling = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_culling_mode(ui32 value)
    {
        m_culling_mode = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_is_stencil_test(bool value)
    {
        m_is_stencil_test = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_stencil_function(ui32 value)
    {
        m_stencil_function = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_stencil_mask_read(i32 value)
    {
        m_stencil_mask_read = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_stencil_mask_write(i32 value)
    {
        m_stencil_mask_write = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_stencil_ref_value(i32 value)
    {
        m_stencil_ref_value = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_back_stencil_op_1(i32 value)
    {
        m_back_stencil_op_1 = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_back_stencil_op_2(i32 value)
    {
        m_back_stencil_op_2 = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_back_stencil_op_3(i32 value)
    {
        m_back_stencil_op_3 = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_front_stencil_op_1(i32 value)
    {
        m_front_stencil_op_1 = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_front_stencil_op_2(i32 value)
    {
        m_front_stencil_op_2 = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_front_stencil_op_3(i32 value)
    {
        m_front_stencil_op_3 = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_is_depth_test(bool value)
    {
        m_is_depth_test = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_is_depth_mask(bool value)
    {
        m_is_depth_mask = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_is_color_mask_r(bool value)
    {
        m_is_color_mask_r = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_is_color_mask_g(bool value)
    {
        m_is_color_mask_g = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_is_color_mask_b(bool value)
    {
        m_is_color_mask_b = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_is_color_mask_a(bool value)
    {
        m_is_color_mask_a = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_is_batching(bool value)
    {
        m_is_batching = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_z_order(ui32 value)
    {
        m_z_order = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::add_blending_parameters(const std::shared_ptr<blending_parameters>& value)
    {
        m_blending_parameters.push_back(value);
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::set_shader(const shader_shared_ptr& value)
    {
        m_shader = value;
        m_is_guid_computed = false;
    }
    
    void material_cached_parameters::add_texture(const texture_shared_ptr& value, e_shader_sampler sampler)
    {
        m_textures[sampler] = value;
        m_is_guid_computed = false;
    }
    
    bool material_cached_parameters::get_is_blending(i32 index) const
    {
        return m_blending_parameters[index]->m_is_blending;
    }
    
    ui32 material_cached_parameters::get_blending_function_source(i32 index) const
    {
        return m_blending_parameters[index]->m_blending_function_source;
    }
    
    ui32 material_cached_parameters::get_blending_function_destination(i32 index) const
    {
        return m_blending_parameters[index]->m_blending_function_destination;
    }
    
    ui32 material_cached_parameters::get_blending_equation(i32 index) const
    {
        return m_blending_parameters[index]->m_blending_equation;
    }
    
    ui32 material_cached_parameters::get_attachment_index(i32 index) const
    {
        return m_blending_parameters[index]->m_attachment_index;
    }
    
    void material_cached_parameters::set_is_blending(i32 index, bool value)
    {
        m_blending_parameters[index]->m_is_blending = value;
    }
    
    void material_cached_parameters::set_blending_function_source(i32 index, ui32 value)
    {
        m_blending_parameters[index]->m_blending_function_source = value;
    }
    
    void material_cached_parameters::set_blending_function_destination(i32 index, ui32 value)
    {
        m_blending_parameters[index]->m_blending_function_destination = value;
    }
    
    void material_cached_parameters::set_blending_equation(i32 index, ui32 value)
    {
        m_blending_parameters[index]->m_blending_equation = value;
    }
    
    void material_cached_parameters::set_attachment_index(i32 index, ui32 value)
    {
        m_blending_parameters[index]->m_attachment_index = value;
    }
    
    std::string material_cached_parameters::get_guid()
    {
        if (!m_is_guid_computed)
        {
            std::stringstream guid_stream;
            
            guid_stream<<m_technique_name;
            guid_stream<<m_is_culling;
            guid_stream<<m_culling_mode;
            
            guid_stream<<m_is_stencil_test;
            guid_stream<<m_stencil_function;
            guid_stream<<m_stencil_mask_read;
            guid_stream<<m_stencil_mask_write;
            guid_stream<<m_stencil_ref_value;
            guid_stream<<m_back_stencil_op_1;
            guid_stream<<m_back_stencil_op_2;
            guid_stream<<m_back_stencil_op_3;
            guid_stream<<m_front_stencil_op_1;
            guid_stream<<m_front_stencil_op_2;
            guid_stream<<m_front_stencil_op_3;
            
            guid_stream<<m_is_depth_test;
            guid_stream<<m_is_depth_mask;
            
            guid_stream<<m_is_color_mask_r;
            guid_stream<<m_is_color_mask_g;
            guid_stream<<m_is_color_mask_b;
            guid_stream<<m_is_color_mask_a;
            
            guid_stream<<m_is_batching;
            
            guid_stream<<m_z_order;
            
            std::for_each(m_blending_parameters.begin(), m_blending_parameters.end(), [&guid_stream](std::shared_ptr<blending_parameters>& iterator){
                guid_stream<<iterator->get_guid();
            });
            
            std::for_each(m_textures.begin(), m_textures.end(), [&guid_stream](std::shared_ptr<texture>& iterator) {
                if (iterator)
                {
                    guid_stream<<iterator->get_guid();
                }
            });
            
            if (m_shader)
            {
                guid_stream<<m_shader->get_guid();
            }
            
            m_guid = guid_stream.str();
            m_is_guid_computed = true;
        }
        return m_guid;
    }
    
    std::shared_ptr<material_cached_parameters> material::m_cached_parameters = nullptr;
    std::once_flag g_cached_parameters_created;
    std::shared_ptr<material_cached_parameters> material::get_cached_parameters()
    {
        std::call_once(g_cached_parameters_created, []{
            m_cached_parameters = std::make_shared<material_cached_parameters>();
            gl::command::enable(gl::constant::depth_test);
            gl::command::depth_mask(true);
        });
        return m_cached_parameters;
    }
    
    material::material() :
    m_parameters(std::make_shared<material_cached_parameters>()),
    m_guid("unknown")
    {
        
    }
    
    material::~material()
    {
        
    }
    
    material_shared_ptr material::construct(const std::shared_ptr<material_configuration> &configuration)
    {
        material_shared_ptr material = std::make_shared<gb::material>();
        assert(configuration);
        material->m_parameters->set_technique_name(configuration->get_technique_name());
        
        material->set_culling(configuration->get_culling());
        material->set_culling_mode(configuration->get_culling_mode());
        
        const auto blending_configurations = configuration->get_blendings_configurations();
        
        i32 blending_configuration_index = 0;
        for (const auto& blending_configuration_it : blending_configurations)
        {
            const auto blending_configuration = std::static_pointer_cast<gb::blending_configuration>(blending_configuration_it);
            const auto blending_parameters = std::make_shared<gb::blending_parameters>();
            material->m_parameters->add_blending_parameters(blending_parameters);
            blending_parameters->set_attachment_index(blending_configuration->get_attachment_index());
            material->set_blending(blending_configuration->get_is_enabled(), blending_configuration_index);
            material->set_blending_function_source(blending_configuration->get_blending_function_source(), blending_configuration_index);
            material->set_blending_function_destination(blending_configuration->get_blending_function_destination(), blending_configuration_index);
            material->set_blending_equation(blending_configuration->get_blending_equation(), blending_configuration_index);
            blending_configuration_index++;
        }
        assert(material->m_parameters->get_blending_parameters().size() > 0);
        
        std::vector<std::shared_ptr<blending_parameters>> blending_parameters = material->m_parameters->get_blending_parameters();
        std::sort(blending_parameters.begin(), blending_parameters.end(), [](const std::shared_ptr<gb::blending_parameters>& a, const std::shared_ptr<gb::blending_parameters>& b) -> bool {
            return a->get_attachment_index() < b->get_attachment_index();
        });
        
        material->set_stencil_test(configuration->get_stencil_test());
        material->set_stencil_function(configuration->get_stencil_function());
        material->set_back_stencil_op_1(configuration->get_back_stencil_op_1());
        material->set_back_stencil_op_2(configuration->get_back_stencil_op_2());
        material->set_back_stencil_op_3(configuration->get_back_stencil_op_3());
        material->set_front_stencil_op_1(configuration->get_front_stencil_op_1());
        material->set_front_stencil_op_2(configuration->get_front_stencil_op_2());
        material->set_front_stencil_op_3(configuration->get_front_stencil_op_3());
        material->set_stencil_mask_read(configuration->get_stencil_mask_read());
        material->set_stencil_mask_write(configuration->get_stencil_mask_write());
        material->set_stencil_ref_value(configuration->get_stencil_ref_value());
        
        material->set_depth_test(configuration->get_depth_test());
        material->set_depth_mask(configuration->get_depth_mask());
        
        material->set_is_batching(configuration->get_is_batching());
        
        material->set_z_order(configuration->get_z_order());
        
        material->set_color_mask_r(configuration->get_color_mask_r());
        material->set_color_mask_g(configuration->get_color_mask_g());
        material->set_color_mask_b(configuration->get_color_mask_b());
        material->set_color_mask_a(configuration->get_color_mask_a());
        
        material->update_guid();
        
#if USED_GRAPHICS_API == METAL_API
        
        material->m_render_encoder = std::make_shared<mtl_render_encoder>();
        material->m_mvp_uniforms_buffer = std::make_shared<mtl_buffer>(sizeof(shader_mvp_uniforms));
        
#endif
        
        return material;
    }
    
    void material::update_guid()
    {
        std::stringstream guid_string_stream;
        guid_string_stream<<m_parameters->get_guid();
        m_guid = guid_string_stream.str();
    }
    
    const std::string& material::get_guid() const
    {
        return m_guid;
    }
    
    void material::set_shader(const material_shared_ptr& material,
                              const std::shared_ptr<material_configuration>& configuration,
                              const resource_accessor_shared_ptr& resource_accessor, bool force)
    {
        std::shared_ptr<shader_configuration> shader_configuration =
        std::static_pointer_cast<gb::shader_configuration>(configuration->get_shader_configuration());
        assert(shader_configuration != nullptr);
        shader_shared_ptr shader = resource_accessor->get_resource<gb::shader, gb::shader_loading_operation>(shader_configuration->get_filename(), force);
        material->set_shader(shader);
    }
    
    void material::set_textures(const material_shared_ptr& material,
                                const std::shared_ptr<material_configuration>& configuration,
                                const resource_accessor_shared_ptr& resource_accessor, bool force)
    {
        for(const auto& iterator : configuration->get_textures_configurations())
        {
            std::shared_ptr<texture_configuration> texture_configuration =
            std::static_pointer_cast<gb::texture_configuration>(iterator);
            assert(texture_configuration != nullptr);
            
            texture_shared_ptr texture = nullptr;
            std::string texture_filename = texture_configuration->get_texture_filename().length() != 0 ?
            texture_configuration->get_texture_filename() : texture_configuration->get_render_technique_name();
            if (!texture_configuration->get_cubemap())
            {
                texture = resource_accessor->get_resource<gb::texture, gb::texture_loading_operation>(texture_filename, force);
            }
            else
            {
                texture = resource_accessor->get_resource<gb::cubemap_texture, gb::cubemap_texture_loading_operation>(texture_filename, force);
            }
            
            assert(texture != nullptr);
            texture->set_wrap_mode(texture_configuration->get_wrap_mode());
            texture->set_mag_filter(texture_configuration->get_mag_filter());
            texture->set_min_filter(texture_configuration->get_min_filter());
            assert(texture_configuration->get_sampler_index() >= 0 &&
                   texture_configuration->get_sampler_index() < e_shader_sampler_max);
            material->set_texture(texture, static_cast<e_shader_sampler>(texture_configuration->get_sampler_index()));
        }
    }
    
    bool material::is_culling() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_is_culling();
    }
    
    std::string material::get_technique_name() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_technique_name();
    }
    
    ui32 material::get_culling_mode() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_culling_mode();
    }
    
    bool material::is_blending() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_blending_parameters().at(0)->get_is_blending();
    }
    
    ui32 material::get_blending_function_source() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_blending_parameters().at(0)->get_blending_function_source();
    }
    
    ui32 material::get_blending_function_destination() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_blending_parameters().at(0)->get_blending_function_destination();
    }
    
    ui32 material::get_blending_equation() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_blending_parameters().at(0)->get_blending_equation();
    }
    
    bool material::is_stencil_test() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_is_stencil_test();
    }
    
    ui32 material::get_stencil_function() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_stencil_function();
    }
    
    i32 material::get_stencil_mask_read() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_stencil_mask_read();
    }
    
    i32 material::get_stencil_mask_write() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_stencil_mask_write();
    }
    
    i32 material::get_stencil_ref_value() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_stencil_ref_value();
    }
    
    i32 material::get_back_stencil_op_1() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_back_stencil_op_1();
    }
    
    i32 material::get_back_stencil_op_2() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_back_stencil_op_2();
    }
    
    i32 material::get_back_stencil_op_3() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_back_stencil_op_3();
    }
    
    i32 material::get_front_stencil_op_1() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_front_stencil_op_1();
    }
    
    i32 material::get_front_stencil_op_2() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_front_stencil_op_2();
    }
    
    i32 material::get_front_stencil_op_3() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_front_stencil_op_3();
    }
    
    bool material::is_depth_test() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_is_depth_test();
    }
    
    bool material::is_depth_mask() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_is_depth_mask();
    }
    
    bool  material::is_color_mask_r() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_is_color_mask_r();
    }
    
    bool  material::is_color_mask_g() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_is_color_mask_g();
    }
    
    bool  material::is_color_mask_b() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_is_color_mask_b();
    }
    
    bool  material::is_color_mask_a() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_is_color_mask_a();
    }
    
    bool material::get_is_batching() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_is_batching();
    }
    
    ui32 material::get_z_order() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_z_order();
    }
    
    shader_shared_ptr material::get_shader() const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_shader();
    }
    
    std::shared_ptr<texture> material::get_texture(e_shader_sampler sampler) const
    {
        assert(m_parameters != nullptr);
        return m_parameters->get_textures().at(sampler);
    }
    
    e_shader_sampler material::get_sampler_index(const std::shared_ptr<texture>& texture) const
    {
        assert(m_parameters != nullptr);
        for(ui32 i = 0; i < m_parameters->get_textures().size(); ++i)
        {
            if(texture == m_parameters->get_textures().at(i))
            {
                return static_cast<e_shader_sampler>(i);
            }
        }
        assert(false);
        return e_shader_sampler_01;
    }
    
    void material::set_culling(bool value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_is_culling(value);
    }
    
    void material::set_culling_mode(ui32 value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_culling_mode(value);
    }
    
    void material::set_blending(bool value, ui32 attachment_index)
    {
        assert(m_parameters != nullptr);
        assert(m_parameters->get_blending_parameters().size() > attachment_index);
        m_parameters->get_blending_parameters().at(attachment_index)->set_is_blending(value);
    }
    
    void material::set_blending_function_source(ui32 value, ui32 attachment_index)
    {
        assert(m_parameters != nullptr);
        assert(m_parameters->get_blending_parameters().size() > attachment_index);
        m_parameters->get_blending_parameters().at(attachment_index)->set_blending_function_source(value);
    }
    
    void material::set_blending_function_destination(ui32 value, ui32 attachment_index)
    {
        assert(m_parameters != nullptr);
        assert(m_parameters->get_blending_parameters().size() > attachment_index);
        m_parameters->get_blending_parameters().at(attachment_index)->set_blending_function_destination(value);
    }
    
    void material::set_blending_equation(ui32 value, ui32 attachment_index)
    {
        assert(m_parameters);
        assert(m_parameters->get_blending_parameters().size() > attachment_index);
        m_parameters->get_blending_parameters().at(attachment_index)->set_blending_equation(value);
    }
    
    void material::set_stencil_test(bool value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_is_stencil_test(value);
    }
    
    void material::set_stencil_function(ui32 value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_stencil_function(value);
    }
    
    void material::set_stencil_mask_read(i32 value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_stencil_mask_read(value);
    }
    
    void material::set_stencil_mask_write(i32 value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_stencil_mask_write(value);
    }
    
    void material::set_stencil_ref_value(i32 value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_stencil_ref_value(value);
    }
    
    void material::set_back_stencil_op_1(i32 value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_back_stencil_op_1(value);
    }
    
    void material::set_back_stencil_op_2(i32 value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_back_stencil_op_2(value);
    }
    
    void material::set_back_stencil_op_3(i32 value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_back_stencil_op_3(value);
    }
    
    void material::set_front_stencil_op_1(i32 value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_front_stencil_op_1(value);
    }
    
    void material::set_front_stencil_op_2(i32 value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_front_stencil_op_2(value);
    }
    
    void material::set_front_stencil_op_3(i32 value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_front_stencil_op_3(value);
    }
    
    void material::set_depth_test(bool value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_is_depth_test(value);
    }
    
    void material::set_depth_mask(bool value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_is_depth_mask(value);
    }
    
    void material::set_color_mask_r(bool value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_is_color_mask_r(value);
    }
    
    void material::set_color_mask_g(bool value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_is_color_mask_g(value);
    }
    
    void material::set_color_mask_b(bool value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_is_color_mask_b(value);
    }
    
    void material::set_color_mask_a(bool value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_is_color_mask_a(value);
    }
    
    void material::set_is_batching(bool value)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_is_batching(value);
    }
    
    void material::set_z_order(ui32 z_order)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_z_order(z_order);
    }
    
    void material::set_shader(const shader_shared_ptr& shader)
    {
        assert(m_parameters != nullptr);
        m_parameters->set_shader(shader);
        material::update_guid();
    }
    
    void material::set_texture(const texture_shared_ptr& texture,
                               e_shader_sampler sampler)
    {
        assert(texture != nullptr);
        assert(m_parameters != nullptr);
        m_parameters->add_texture(texture, sampler);
        material::update_guid();
    }
    
    void material::set_custom_shader_uniform(const glm::mat4& matrix, const std::string& uniform)
    {
        const auto& iterator = m_custom_shader_uniforms.find(uniform);
        std::shared_ptr<shader_uniform> current_uniform = nullptr;
        if(iterator != m_custom_shader_uniforms.end())
        {
            current_uniform = iterator->second;
        }
        else
        {
            current_uniform = std::make_shared<shader_uniform>(e_uniform_type_mat4);
            m_custom_shader_uniforms.insert(std::make_pair(uniform, current_uniform));
        }
        current_uniform->set(matrix);
    }
    
    void material::set_custom_shader_uniform(glm::mat4* matrices, i32 size, const std::string& uniform)
    {
        const auto& iterator = m_custom_shader_uniforms.find(uniform);
        std::shared_ptr<shader_uniform> current_uniform = nullptr;
        if(iterator != m_custom_shader_uniforms.end())
        {
            current_uniform = iterator->second;
        }
        else
        {
            current_uniform = std::make_shared<shader_uniform>(e_uniform_type_mat4_array, size);
            m_custom_shader_uniforms.insert(std::make_pair(uniform, current_uniform));
        }
        current_uniform->set(matrices, size);
    }
    
    
    void material::set_custom_shader_uniform(const glm::mat3& matrix, const std::string& uniform)
    {
        const auto& iterator = m_custom_shader_uniforms.find(uniform);
        std::shared_ptr<shader_uniform> current_uniform = nullptr;
        if(iterator != m_custom_shader_uniforms.end())
        {
            current_uniform = iterator->second;
        }
        else
        {
            current_uniform = std::make_shared<shader_uniform>(e_uniform_type_mat3);
            m_custom_shader_uniforms.insert(std::make_pair(uniform, current_uniform));
        }
        current_uniform->set(matrix);
    }
    
    void material::set_custom_shader_uniform(const glm::vec4& vector, const std::string& uniform)
    {
        const auto& iterator = m_custom_shader_uniforms.find(uniform);
        std::shared_ptr<shader_uniform> current_uniform = nullptr;
        if(iterator != m_custom_shader_uniforms.end())
        {
            current_uniform = iterator->second;
        }
        else
        {
            current_uniform = std::make_shared<shader_uniform>(e_uniform_type_vec4);
            m_custom_shader_uniforms.insert(std::make_pair(uniform, current_uniform));
        }
        current_uniform->set(vector);
    }
    
    void material::set_custom_shader_uniform(glm::vec4* vectors, i32 size, const std::string& uniform)
    {
        const auto& iterator = m_custom_shader_uniforms.find(uniform);
        std::shared_ptr<shader_uniform> current_uniform = nullptr;
        if(iterator != m_custom_shader_uniforms.end())
        {
            current_uniform = iterator->second;
        }
        else
        {
            current_uniform = std::make_shared<shader_uniform>(e_uniform_type_vec4_array, size);
            m_custom_shader_uniforms.insert(std::make_pair(uniform, current_uniform));
        }
        current_uniform->set(vectors, size);
    }
    
    void material::set_custom_shader_uniform(const glm::vec3& vector, const std::string& uniform)
    {
        const auto& iterator = m_custom_shader_uniforms.find(uniform);
        std::shared_ptr<shader_uniform> current_uniform = nullptr;
        if(iterator != m_custom_shader_uniforms.end())
        {
            current_uniform = iterator->second;
        }
        else
        {
            current_uniform = std::make_shared<shader_uniform>(e_uniform_type_vec3);
            m_custom_shader_uniforms.insert(std::make_pair(uniform, current_uniform));
        }
        current_uniform->set(vector);
    }
    
    void material::set_custom_shader_uniform(const glm::vec2& vector, const std::string& uniform)
    {
        const auto& iterator = m_custom_shader_uniforms.find(uniform);
        std::shared_ptr<shader_uniform> current_uniform = nullptr;
        if(iterator != m_custom_shader_uniforms.end())
        {
            current_uniform = iterator->second;
        }
        else
        {
            current_uniform = std::make_shared<shader_uniform>(e_uniform_type_vec2);
            m_custom_shader_uniforms.insert(std::make_pair(uniform, current_uniform));
        }
        current_uniform->set(vector);
    }
    
    void material::set_custom_shader_uniform(f32 value, const std::string& uniform)
    {
        const auto& iterator = m_custom_shader_uniforms.find(uniform);
        std::shared_ptr<shader_uniform> current_uniform = nullptr;
        if(iterator != m_custom_shader_uniforms.end())
        {
            current_uniform = iterator->second;
        }
        else
        {
            current_uniform = std::make_shared<shader_uniform>(e_uniform_type_f32);
            m_custom_shader_uniforms.insert(std::make_pair(uniform, current_uniform));
        }
        current_uniform->set(value);
    }
    
    void material::set_custom_shader_uniform(i32 value, const std::string& uniform)
    {
        const auto& iterator = m_custom_shader_uniforms.find(uniform);
        std::shared_ptr<shader_uniform> current_uniform = nullptr;
        if(iterator != m_custom_shader_uniforms.end())
        {
            current_uniform = iterator->second;
        }
        else
        {
            current_uniform = std::make_shared<shader_uniform>(e_uniform_type_i32);
            m_custom_shader_uniforms.insert(std::make_pair(uniform, current_uniform));
        }
        current_uniform->set(value);
    }
    
    void material::set_custom_shader_uniforms(const std::unordered_map<std::string, std::shared_ptr<shader_uniform>>& uniforms)
    {
        for (auto uniform_it : uniforms)
        {
            switch(uniform_it.second->get_type())
            {
                case e_uniform_type_mat4:
                {
                    set_custom_shader_uniform(uniform_it.second->get_mat4(),
                                              uniform_it.first);
                }
                    break;
                    
                case e_uniform_type_mat4_array:
                {
                }
                    break;
                    
                case e_uniform_type_mat3:
                {
                    set_custom_shader_uniform(uniform_it.second->get_mat3(),
                                              uniform_it.first);
                }
                    break;
                    
                case e_uniform_type_vec4:
                {
                    set_custom_shader_uniform(uniform_it.second->get_vec4(),
                                              uniform_it.first);
                }
                    break;
                    
                case e_uniform_type_vec4_array:
                {
                }
                    break;
                    
                case e_uniform_type_vec3:
                {
                    set_custom_shader_uniform(uniform_it.second->get_vec3(),
                                              uniform_it.first);
                }
                    break;
                    
                case e_uniform_type_vec2:
                {
                    set_custom_shader_uniform(uniform_it.second->get_vec2(),
                                              uniform_it.first);
                }
                    break;
                    
                case e_uniform_type_f32:
                {
                    set_custom_shader_uniform(uniform_it.second->get_f32(),
                                              uniform_it.first);
                }
                    break;
                    
                case e_uniform_type_i32:
                {
                    set_custom_shader_uniform(uniform_it.second->get_i32(),
                                              uniform_it.first);
                }
                    break;
                    
                default:
                {
                    assert(false);
                }
                    break;
            }
        }
    }
    
    const std::map<std::string, std::shared_ptr<shader_uniform>>& material::get_custom_uniforms() const
    {
        return m_custom_shader_uniforms;
    }
    
    void material::bind_custom_shader_uniforms()
    {
        std::shared_ptr<shader_uniform> current_uniform = nullptr;
        for(const auto& iterator : m_custom_shader_uniforms)
        {
            current_uniform = iterator.second;
            assert(current_uniform);
            
            switch (current_uniform->get_type())
            {
                case e_uniform_type_mat4:
                {
                    m_parameters->get_shader()->set_custom_mat4(current_uniform->get_mat4(), iterator.first);
                }
                    break;
                    
                case e_uniform_type_mat4_array:
                {
                    m_parameters->get_shader()->set_custom_mat4_array(current_uniform->get_mat4_array(),
                                                                      current_uniform->get_array_size(),
                                                                      iterator.first);
                }
                    break;
                    
                case e_uniform_type_mat3:
                {
                    m_parameters->get_shader()->set_custom_mat3(current_uniform->get_mat3(), iterator.first);
                }
                    break;
                    
                case e_uniform_type_vec4:
                {
                    m_parameters->get_shader()->set_custom_vec4(current_uniform->get_vec4(), iterator.first);
                }
                    break;
                    
                case e_uniform_type_vec4_array:
                {
                    m_parameters->get_shader()->set_custom_vec4_array(current_uniform->get_vec4_array(),
                                                                      current_uniform->get_array_size(),
                                                                      iterator.first);
                }
                    break;
                    
                case e_uniform_type_vec3:
                {
                    m_parameters->get_shader()->set_custom_vec3(current_uniform->get_vec3(), iterator.first);
                }
                    break;
                    
                case e_uniform_type_vec2:
                {
                    m_parameters->get_shader()->set_custom_vec2(current_uniform->get_vec2(), iterator.first);
                }
                    break;
                    
                case e_uniform_type_f32:
                {
                    m_parameters->get_shader()->set_custom_f32(current_uniform->get_f32(), iterator.first);
                }
                    break;
                    
                case e_uniform_type_i32:
                {
                    m_parameters->get_shader()->set_custom_i32(current_uniform->get_i32(), iterator.first);
                }
                    break;
                    
                default:
                {
                    assert(false);
                }
                    break;
            }
        }
    }
    
#if USED_GRAPHICS_API == VULKAN_API
    
    void material::bind(const VkPipelineVertexInputStateCreateInfo& vertex_input_state)
    
#elif USED_GRAPHICS_API == METAL_API
    
    void material::bind(const mtl_vertex_descriptor_shared_ptr& vertex_descriptor)
    
#else
    
    void material::bind()
    
#endif
    
    {
        assert(m_parameters != nullptr);
        assert(m_parameters->get_shader() != nullptr);
        
        const auto cached_parameters = material::get_cached_parameters();
        const auto is_same_technique = cached_parameters->get_technique_name() == m_parameters->get_technique_name();
        
#if USED_GRAPHICS_API == VULKAN_API
        
        if (!m_is_pipeline_constructed)
        {
            construct_pipeline(vertex_input_state);
        }
        
#elif USED_GRAPHICS_API == METAL_API
        
        if (!m_render_pipeline_state)
        {
            m_render_pipeline_state = mtl_render_pipeline_state::construct(m_parameters,
                                                                           vertex_descriptor);
        }
        
        if (!m_depth_stencil_state)
        {
            m_depth_stencil_state = mtl_depth_stencil_state::construct(m_parameters);
        }
        
        m_render_encoder->bind(m_parameters->get_technique_name());
        m_render_encoder->set_render_pipeline_state(m_render_pipeline_state);
        m_render_encoder->set_depth_stencil_state(m_depth_stencil_state);
        
#endif
        
        m_parameters->get_shader()->bind();
        
        for(ui32 i = 0; i < e_shader_sampler_max; ++i)
        {
            const auto texture = m_parameters->get_textures().at(i);
            if(texture != nullptr)
            {
                m_parameters->get_shader()->set_texture(texture, static_cast<e_shader_sampler>(i));
                
#if USED_GRAPHICS_API == METAL_API
                
                if (texture->get_mtl_texture_id())
                {
                    if (cached_parameters->get_textures().at(i) != texture || !is_same_technique)
                    {
                        m_render_encoder->set_texture(texture->get_mtl_texture_id(), i);
                        cached_parameters->add_texture(texture, static_cast<e_shader_sampler>(i));
                    }
                }

#elif USED_GRAPHICS_API == VULKAN_API

				(void)texture;
                
#endif
                
            }
        }
        
        if (m_parameters->get_is_depth_test() &&
            cached_parameters->get_is_depth_test() != m_parameters->get_is_depth_test())
        {
            gl::command::enable(gl::constant::depth_test);
            gl::command::depth_function(gl::constant::lequal);
            cached_parameters->set_is_depth_test(m_parameters->get_is_depth_test());
        }
        else if (cached_parameters->get_is_depth_test() != m_parameters->get_is_depth_test())
        {
            gl::command::disable(gl::constant::depth_test);
            cached_parameters->set_is_depth_test(m_parameters->get_is_depth_test());
        }
        
        if(m_parameters->get_is_depth_mask() &&
           cached_parameters->get_is_depth_mask() != m_parameters->get_is_depth_mask())
        {
            gl::command::depth_mask(gl::constant::yes);
            cached_parameters->set_is_depth_mask(m_parameters->get_is_depth_mask());
        }
        else if(cached_parameters->get_is_depth_mask() != m_parameters->get_is_depth_mask())
        {
            gl::command::depth_mask(gl::constant::no);
            cached_parameters->set_is_depth_mask(m_parameters->get_is_depth_mask());
        }
        
        if(m_parameters->get_is_culling() &&
           (cached_parameters->get_is_culling() != m_parameters->get_is_culling() ||
            !is_same_technique))
        {
            gl::command::enable(gl::constant::cull_face);
            cached_parameters->set_is_culling(m_parameters->get_is_culling());
        }
        else if(cached_parameters->get_is_culling() != m_parameters->get_is_culling() ||
                !is_same_technique)
        {
            gl::command::disable(gl::constant::cull_face);
            cached_parameters->set_is_culling(m_parameters->get_is_culling());
            
#if USED_GRAPHICS_API == METAL_API
            
            m_render_encoder->set_cull_mode_none();
            
#endif
        }
        
        if(m_parameters->get_is_culling() &&
           (cached_parameters->get_culling_mode() != m_parameters->get_culling_mode() ||
            !is_same_technique))
        {
            gl::command::cull_face(m_parameters->get_culling_mode());
            cached_parameters->set_culling_mode(m_parameters->get_culling_mode());
            
#if USED_GRAPHICS_API == METAL_API
            
            m_render_encoder->set_cull_mode(m_parameters->get_culling_mode());
            
#endif
        }
        
        if(m_parameters->get_is_blending(0) &&
           cached_parameters->get_is_blending(0) != m_parameters->get_is_blending(0))
        {
            gl::command::enable(gl::constant::blend);
            cached_parameters->set_is_blending(0, m_parameters->get_is_blending(0));
        }
        else if(cached_parameters->get_is_blending(0) != m_parameters->get_is_blending(0))
        {
            gl::command::disable(gl::constant::blend);
            cached_parameters->set_is_blending(0, m_parameters->get_is_blending(0));
        }
        
        if(cached_parameters->get_blending_function_source(0) !=
           m_parameters->get_blending_function_source(0) ||
           cached_parameters->get_blending_function_destination(0) !=
           m_parameters->get_blending_function_destination(0))
        {
            gl::command::blend_function(m_parameters->get_blending_function_source(0), m_parameters->get_blending_function_destination(0));
            cached_parameters->set_blending_function_source(0, m_parameters->get_blending_function_source(0));
            cached_parameters->set_blending_function_destination(0, m_parameters->get_blending_function_destination(0));
        }
        
        if(cached_parameters->get_blending_equation(0) != m_parameters->get_blending_equation(0))
        {
            gl::command::blend_equation(m_parameters->get_blending_equation(0));
            cached_parameters->set_blending_equation(0, m_parameters->get_blending_equation(0));
        }
        
        if(m_parameters->get_is_stencil_test() &&
           (cached_parameters->get_is_stencil_test() != m_parameters->get_is_stencil_test() ||
            !is_same_technique))
        {
            gl::command::enable(gl::constant::stencil_test);
            cached_parameters->set_is_stencil_test(m_parameters->get_is_stencil_test());
            
#if USED_GRAPHICS_API == METAL_API
            
            m_render_encoder->set_stencil_value(m_parameters->get_stencil_ref_value());
            
#endif
        }
        else if(cached_parameters->get_is_stencil_test() != m_parameters->get_is_stencil_test())
        {
            gl::command::disable(gl::constant::stencil_test);
            cached_parameters->set_is_stencil_test(m_parameters->get_is_stencil_test());
        }
        
        if(m_parameters->get_is_color_mask_r() != cached_parameters->get_is_color_mask_r() ||
           m_parameters->get_is_color_mask_g() != cached_parameters->get_is_color_mask_g() ||
           m_parameters->get_is_color_mask_b() != cached_parameters->get_is_color_mask_b() ||
           m_parameters->get_is_color_mask_a() != cached_parameters->get_is_color_mask_a())
        {
            cached_parameters->set_is_color_mask_r(m_parameters->get_is_color_mask_r());
            cached_parameters->set_is_color_mask_g(m_parameters->get_is_color_mask_g());
            cached_parameters->set_is_color_mask_b(m_parameters->get_is_color_mask_b());
            cached_parameters->set_is_color_mask_a(m_parameters->get_is_color_mask_a());
            
            gl::command::color_mask(m_parameters->get_is_color_mask_r(), m_parameters->get_is_color_mask_g(),
                                    m_parameters->get_is_color_mask_b(), m_parameters->get_is_color_mask_a());
        }
        
        material::bind_custom_shader_uniforms();
        
#if USED_GRAPHICS_API == VULKAN_API
        
        ui32 current_image_index = vk_device::get_instance()->get_current_image_index();
        VkCommandBuffer draw_cmd_buffer = vk_device::get_instance()->get_draw_cmd_buffer(current_image_index);
        
        vkCmdBindPipeline(draw_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
        const auto shader = m_parameters->get_shader();
        
#endif
        
        cached_parameters->set_technique_name(m_parameters->get_technique_name());
        
    }
    
    void material::unbind()
    {
        assert(m_parameters != nullptr);
        assert(m_parameters->get_shader() != nullptr);
        m_parameters->get_shader()->unbind();
        
#if USED_GRAPHICS_API == METAL_API
        
        m_render_encoder->unbind(m_parameters->get_technique_name());
        
#endif
        
    }
    
#if USED_GRAPHICS_API == VULKAN_API
    
    void material::construct_pipeline(const VkPipelineVertexInputStateCreateInfo& vertex_input_state)
    {
        m_input_assembly_state = vk_initializers::pipeline_input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
		VkCullModeFlags cull_mode = VK_CULL_MODE_NONE;
		if (m_parameters->get_is_culling())
		{
			cull_mode = m_parameters->get_culling_mode() == gl::constant::front ? VK_CULL_MODE_FRONT_BIT : VK_CULL_MODE_BACK_BIT;
		}
		m_rasterization_state = vk_initializers::pipeline_rasterization_state_create_info(VK_POLYGON_MODE_FILL, cull_mode, VK_FRONT_FACE_CLOCKWISE, 0);

		VkColorComponentFlags color_write_mask = 0;
		color_write_mask |= m_parameters->get_is_color_mask_r() ? VK_COLOR_COMPONENT_R_BIT : 0;
		color_write_mask |= m_parameters->get_is_color_mask_g() ? VK_COLOR_COMPONENT_G_BIT : 0;
		color_write_mask |= m_parameters->get_is_color_mask_b() ? VK_COLOR_COMPONENT_B_BIT : 0;
		color_write_mask |= m_parameters->get_is_color_mask_a() ? VK_COLOR_COMPONENT_A_BIT : 0;
		auto get_blend_factor = [](ui32 factor) {
			if (factor == gl::constant::zero) return VK_BLEND_FACTOR_ZERO;
			if (factor == gl::constant::one) return VK_BLEND_FACTOR_ONE;
			if (factor == gl::constant::src_alpha) return VK_BLEND_FACTOR_SRC_ALPHA;
			if (factor == gl::constant::one_minus_src_alpha) return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			if (factor == gl::constant::one_minus_src_color) return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			if (factor == gl::constant::one_minus_dst_color) return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			if (factor == gl::constant::one_minus_dst_alpha) return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			if (factor == gl::constant::dst_alpha) return VK_BLEND_FACTOR_DST_ALPHA;
			if (factor == gl::constant::constant_alpha) return VK_BLEND_FACTOR_CONSTANT_ALPHA;
			return VK_BLEND_FACTOR_SRC_COLOR;
		};
		auto get_blend_operation = [](ui32 operation) {
			if (operation == gl::constant::func_min) return VK_BLEND_OP_MIN;
			if (operation == gl::constant::func_max) return VK_BLEND_OP_MAX;
			return VK_BLEND_OP_ADD;
		};
        const auto color_attachments_count = vk_render_pass_descriptor::get_current_color_attachments_count();
        assert(color_attachments_count != 0);
		std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments(color_attachments_count);
		const auto& blending_parameters = m_parameters->get_blending_parameters();
		for (ui32 i = 0; i < color_attachments_count; ++i)
		{
			const auto parameters = i < blending_parameters.size() ? blending_parameters.at(i) : blending_parameters.at(0);
			auto& attachment = color_blend_attachments[i];
			attachment = vk_initializers::pipeline_color_blend_attachment_state(color_write_mask, parameters->get_is_blending());
			attachment.srcColorBlendFactor = get_blend_factor(parameters->get_blending_function_source());
			attachment.dstColorBlendFactor = get_blend_factor(parameters->get_blending_function_destination());
			attachment.srcAlphaBlendFactor = attachment.srcColorBlendFactor;
			attachment.dstAlphaBlendFactor = attachment.dstColorBlendFactor;
			attachment.colorBlendOp = get_blend_operation(parameters->get_blending_equation());
			attachment.alphaBlendOp = attachment.colorBlendOp;
		}
        m_color_blend_state = vk_initializers::pipeline_color_blend_state_create_info(color_attachments_count, color_blend_attachments.data());
		m_depth_stencil_state = vk_initializers::pipeline_depth_stencil_state_create_info(m_parameters->get_is_depth_test(), m_parameters->get_is_depth_mask(), VK_COMPARE_OP_LESS_OR_EQUAL);
		if (m_parameters->get_is_stencil_test())
		{
			auto get_compare_operation = [](ui32 operation) {
				if (operation == gl::constant::equal) return VK_COMPARE_OP_EQUAL;
				if (operation == gl::constant::notequal) return VK_COMPARE_OP_NOT_EQUAL;
				if (operation == gl::constant::less) return VK_COMPARE_OP_LESS;
				if (operation == gl::constant::less_equal || operation == gl::constant::lequal) return VK_COMPARE_OP_LESS_OR_EQUAL;
				return VK_COMPARE_OP_ALWAYS;
			};
			auto get_stencil_operation = [](ui32 operation) {
				if (operation == gl::constant::stencil_op_zero) return VK_STENCIL_OP_ZERO;
				if (operation == gl::constant::stencil_op_replace) return VK_STENCIL_OP_REPLACE;
				if (operation == gl::constant::stencil_op_inc_clamp) return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
				if (operation == gl::constant::stencil_op_dec_clamp) return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
				if (operation == gl::constant::stencil_op_inv) return VK_STENCIL_OP_INVERT;
				if (operation == gl::constant::stencil_op_inc_wrap) return VK_STENCIL_OP_INCREMENT_AND_WRAP;
				if (operation == gl::constant::stencil_op_dec_wrap) return VK_STENCIL_OP_DECREMENT_AND_WRAP;
				return VK_STENCIL_OP_KEEP;
			};
			m_depth_stencil_state.stencilTestEnable = VK_TRUE;
			m_depth_stencil_state.front.compareOp = get_compare_operation(m_parameters->get_stencil_function());
			m_depth_stencil_state.front.failOp = get_stencil_operation(m_parameters->get_front_stencil_op_1());
			m_depth_stencil_state.front.depthFailOp = get_stencil_operation(m_parameters->get_front_stencil_op_2());
			m_depth_stencil_state.front.passOp = get_stencil_operation(m_parameters->get_front_stencil_op_3());
			m_depth_stencil_state.front.compareMask = m_parameters->get_stencil_mask_read();
			m_depth_stencil_state.front.writeMask = m_parameters->get_stencil_mask_write();
			m_depth_stencil_state.front.reference = m_parameters->get_stencil_ref_value();
			m_depth_stencil_state.back.compareOp = get_compare_operation(m_parameters->get_stencil_function());
			m_depth_stencil_state.back.failOp = get_stencil_operation(m_parameters->get_back_stencil_op_1());
			m_depth_stencil_state.back.depthFailOp = get_stencil_operation(m_parameters->get_back_stencil_op_2());
			m_depth_stencil_state.back.passOp = get_stencil_operation(m_parameters->get_back_stencil_op_3());
			m_depth_stencil_state.back.compareMask = m_parameters->get_stencil_mask_read();
			m_depth_stencil_state.back.writeMask = m_parameters->get_stencil_mask_write();
			m_depth_stencil_state.back.reference = m_parameters->get_stencil_ref_value();
		}
        
        VkViewport viewport = vk_initializers::viewport(vk_swap_chain::get_instance()->get_swap_chain_extent().width, vk_swap_chain::get_instance()->get_swap_chain_extent().height, 0.f, 1.f);
        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = vk_swap_chain::get_instance()->get_swap_chain_extent();
        
        m_viewport_state = vk_initializers::pipeline_viewport_state_create_info(1, 1, 0);
        m_viewport_state.pViewports = &viewport;
		m_viewport_state.pScissors = &scissor;
		m_dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		m_dynamic_state = {};
		m_dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		m_dynamic_state.pDynamicStates = m_dynamic_states.data();
		m_dynamic_state.dynamicStateCount = static_cast<ui32>(m_dynamic_states.size());
        
        m_multisampling_state = vk_initializers::pipeline_multisample_state_create_info(VK_SAMPLE_COUNT_1_BIT, 0);
        
        const auto shader = m_parameters->get_shader();
        VkPipelineShaderStageCreateInfo shader_stages[] = { shader->get_vs_shader_stage(), shader->get_fs_shader_stage() };
        
        const auto render_pass = vk_render_pass_descriptor::get_current_render_pass();
        assert(render_pass != VK_NULL_HANDLE);
        m_graphics_pipeline = vk_initializers::pipeline_create_info(shader->get_pipeline_layout(), render_pass, 0);
        m_graphics_pipeline.stageCount = 2;
        m_graphics_pipeline.pStages = shader_stages;
        m_graphics_pipeline.pVertexInputState = &vertex_input_state;
        m_graphics_pipeline.pInputAssemblyState = &m_input_assembly_state;
        m_graphics_pipeline.pViewportState = &m_viewport_state;
        m_graphics_pipeline.pRasterizationState = &m_rasterization_state;
        m_graphics_pipeline.pMultisampleState = &m_multisampling_state;
        m_graphics_pipeline.pColorBlendState = &m_color_blend_state;
        m_graphics_pipeline.pDepthStencilState = &m_depth_stencil_state;
		m_graphics_pipeline.pDynamicState = &m_dynamic_state;
        m_graphics_pipeline.subpass = 0;
        m_graphics_pipeline.basePipelineHandle = VK_NULL_HANDLE;
        
        VK_CHECK(vkCreateGraphicsPipelines(vk_device::get_instance()->get_logical_device(), VK_NULL_HANDLE, 1, &m_graphics_pipeline, nullptr, &m_pipeline));
        
        
        //VkDescriptorBufferInfo descriptor_buffer_info = {};
        //descriptor_buffer_info.buffer = uniformBuffer;
        //descriptor_buffer_info.offset = 0;
        //descriptor_buffer_info.range = sizeof(vk_buffer);
        
        m_is_pipeline_constructed = true;
    }
    
#endif
    
#if USED_GRAPHICS_API == METAL_API
    
    mtl_render_encoder_shared_ptr material::get_render_encoder() const
    {
        return m_render_encoder;
    }
    
    mtl_buffer_shared_ptr material::get_mvp_uniforms_buffer() const
    {
        return m_mvp_uniforms_buffer;
    }
    
    mtl_buffer_shared_ptr material::get_custom_uniform_buffer(ui32 size)
    {
        if (!m_custom_uniforms_buffer)
        {
            m_custom_uniforms_buffer = std::make_shared<mtl_buffer>(size);
        }
        assert(m_custom_uniforms_buffer->get_size() == size);
        return m_custom_uniforms_buffer;
    }
    
#endif
    
    }

#endif
