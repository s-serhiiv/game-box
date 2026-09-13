//
//  material.h
//  gbCore
//
//  Created by sergey.sergeev on 8/13/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#pragma once

#include "main_headers.h"

#if USED_GRAPHICS_API != NO_GRAPHICS_API

#include "texture.h"
#include "shader.h"
#include "material_configuration.h"
#include "resource_accessor.h"
#include "declarations.h"

namespace gb
{
    class blending_parameters
    {
    private:
        
        friend class material_cached_parameters;
        
        std::string m_guid;
        bool m_is_guid_computed = false;
        
        bool m_is_blending = false;
        ui32 m_blending_function_source = 0;
        ui32 m_blending_function_destination = 0;
        ui32 m_blending_equation = 0;
        ui32 m_attachment_index = 0;
        
    protected:
        
    public:
        
        blending_parameters() = default;
        ~blending_parameters() = default;
        
        bool get_is_blending() const;
        ui32 get_blending_function_source() const;
        ui32 get_blending_function_destination() const;
        ui32 get_blending_equation() const;
        ui32 get_attachment_index() const;
        
        void set_is_blending(bool value);
        void set_blending_function_source(ui32 value);
        void set_blending_function_destination(ui32 value);
        void set_blending_equation(ui32 value);
        void set_attachment_index(ui32 value);
        
        std::string get_guid();
    };
    
    class material_cached_parameters
    {
    private:
        
        std::string m_guid;
        bool m_is_guid_computed = false;
        
        std::string m_technique_name = "undefined";
        
        bool m_is_culling = false;
        ui32 m_culling_mode = 0;
        
        bool m_is_stencil_test = false;
        ui32 m_stencil_function = 0;
        i32 m_stencil_mask_read = 0;
        i32 m_stencil_mask_write = 0;
        i32 m_stencil_ref_value = 0;
        i32 m_back_stencil_op_1 = 0;
        i32 m_back_stencil_op_2 = 0;
        i32 m_back_stencil_op_3 = 0;
        i32 m_front_stencil_op_1 = 0;
        i32 m_front_stencil_op_2 = 0;
        i32 m_front_stencil_op_3 = 0;
        
        bool m_is_depth_test = false;
        bool m_is_depth_mask = false;
        
        bool m_is_color_mask_r = false;
        bool m_is_color_mask_g = false;
        bool m_is_color_mask_b = false;
        bool m_is_color_mask_a = false;
        
        bool m_is_batching = false;
        
        ui32 m_z_order = 0;
        
        std::vector<std::shared_ptr<blending_parameters>> m_blending_parameters;
        
        shader_shared_ptr m_shader = nullptr;
        std::array<texture_shared_ptr, e_shader_sampler_max> m_textures;
        
    protected:
        
    public:
        
        material_cached_parameters();
        ~material_cached_parameters() = default;
        
        std::string get_technique_name() const;
        void set_technique_name(const std::string& value);
        
        bool get_is_culling() const;
        ui32 get_culling_mode() const;
        
        bool get_is_stencil_test() const;
        ui32 get_stencil_function() const;
        i32 get_stencil_mask_read() const;
        i32 get_stencil_mask_write() const;
        i32 get_stencil_ref_value() const;
        
        i32 get_back_stencil_op_1() const;
        i32 get_back_stencil_op_2() const;
        i32 get_back_stencil_op_3() const;
        
        i32 get_front_stencil_op_1() const;
        i32 get_front_stencil_op_2() const;
        i32 get_front_stencil_op_3() const;
        
        bool get_is_depth_test() const;
        bool get_is_depth_mask() const;
        
        bool get_is_color_mask_r() const;
        bool get_is_color_mask_g() const;
        bool get_is_color_mask_b() const;
        bool get_is_color_mask_a() const;
    
        bool get_is_batching() const;
        
        ui32 get_z_order() const;
        
        const std::vector<std::shared_ptr<blending_parameters>>& get_blending_parameters() const;
        
        shader_shared_ptr get_shader() const;
        const std::array<texture_shared_ptr, e_shader_sampler_max>& get_textures() const;
        
        void set_is_culling(bool value);
        void set_culling_mode(ui32 value);
        
        void set_is_stencil_test(bool value);
        void set_stencil_function(ui32 value);
        void set_stencil_mask_read(i32 value);
        void set_stencil_mask_write(i32 value);
        void set_stencil_ref_value(i32 value);
        
        void set_back_stencil_op_1(i32 value);
        void set_back_stencil_op_2(i32 value);
        void set_back_stencil_op_3(i32 value);
        void set_front_stencil_op_1(i32 value);
        void set_front_stencil_op_2(i32 value);
        void set_front_stencil_op_3(i32 value);
        
        void set_is_depth_test(bool value);
        void set_is_depth_mask(bool value);
        
        void set_is_color_mask_r(bool value);
        void set_is_color_mask_g(bool value);
        void set_is_color_mask_b(bool value);
        void set_is_color_mask_a(bool value);
        
        void set_is_batching(bool value);
        
        void set_z_order(ui32 value);
        
        void add_blending_parameters(const std::shared_ptr<blending_parameters>& value);
        
        void set_shader(const shader_shared_ptr& value);
        void add_texture(const texture_shared_ptr& value, e_shader_sampler sampler);
        
        bool get_is_blending(i32 index) const;
        ui32 get_blending_function_source(i32 index) const;
        ui32 get_blending_function_destination(i32 index) const;
        ui32 get_blending_equation(i32 index) const;
        ui32 get_attachment_index(i32 index) const;
        
        void set_is_blending(i32 index, bool value);
        void set_blending_function_source(i32 index, ui32 value);
        void set_blending_function_destination(i32 index, ui32 value);
        void set_blending_equation(i32 index, ui32 value);
        void set_attachment_index(i32 index, ui32 value);
        
        std::string get_guid();
    };
    
    class material
    {
    private:
        
    protected:
        
        std::map<std::string, std::shared_ptr<shader_uniform>> m_custom_shader_uniforms;
        std::shared_ptr<material_cached_parameters> m_parameters;
        
        std::string m_guid;
        
        static std::shared_ptr<material_cached_parameters> m_cached_parameters;
        
        void bind_custom_shader_uniforms();
        
        void update_guid();

#if USED_GRAPHICS_API == VULKAN_API

		VkPipeline m_pipeline = VK_NULL_HANDLE;
		VkGraphicsPipelineCreateInfo m_graphics_pipeline;
		VkPipelineViewportStateCreateInfo m_viewport_state;
		VkPipelineMultisampleStateCreateInfo m_multisampling_state;
		VkPipelineInputAssemblyStateCreateInfo m_input_assembly_state;
		VkPipelineRasterizationStateCreateInfo m_rasterization_state;
		VkPipelineColorBlendAttachmentState m_color_blend_attachment;
		VkPipelineColorBlendStateCreateInfo m_color_blend_state;
		VkPipelineDepthStencilStateCreateInfo m_depth_stencil_state;
		VkPipelineDynamicStateCreateInfo m_dynamic_state;
		std::array<VkDynamicState, 2> m_dynamic_states;
		std::vector<VkWriteDescriptorSet> m_descriptor_writes;

		bool m_is_pipeline_constructed = false;
		void construct_pipeline(const VkPipelineVertexInputStateCreateInfo& vertex_input_state);

#elif USED_GRAPHICS_API == METAL_API
        
        mtl_render_encoder_shared_ptr m_render_encoder = nullptr;
        mtl_render_pipeline_state_shared_ptr m_render_pipeline_state = nullptr;
        mtl_depth_stencil_state_shared_ptr m_depth_stencil_state = nullptr;
        mtl_buffer_shared_ptr m_mvp_uniforms_buffer = nullptr;
        mtl_buffer_shared_ptr m_custom_uniforms_buffer = nullptr;
        
#endif
        
    public:
        
        material();
        ~material();
        
        static material_shared_ptr construct(const std::shared_ptr<material_configuration>& configuration);
        
        static std::shared_ptr<material_cached_parameters> get_cached_parameters();
        
        static void set_shader(const material_shared_ptr& material,
                               const std::shared_ptr<material_configuration>& configuration,
                               const resource_accessor_shared_ptr& resource_accessor, bool force = false);
        
        static void set_textures(const material_shared_ptr& material,
                                 const std::shared_ptr<material_configuration>& configuration,
                                 const resource_accessor_shared_ptr& resource_accessor, bool force = false);
        
        std::string get_technique_name() const;
        
        bool is_culling() const;
        ui32 get_culling_mode() const;
        
        bool is_blending() const;
        ui32 get_blending_function_source() const;
        ui32 get_blending_function_destination() const;
        ui32 get_blending_equation() const;
        
        bool is_stencil_test() const;
        ui32 get_stencil_function() const;
        i32 get_stencil_mask_read() const;
        i32 get_stencil_mask_write() const;
        i32 get_stencil_ref_value() const;
        
        i32 get_back_stencil_op_1() const;
        i32 get_back_stencil_op_2() const;
        i32 get_back_stencil_op_3() const;
        
        i32 get_front_stencil_op_1() const;
        i32 get_front_stencil_op_2() const;
        i32 get_front_stencil_op_3() const;
        
        bool is_depth_test() const;
        bool is_depth_mask() const;
        
        bool is_color_mask_r() const;
        bool is_color_mask_g() const;
        bool is_color_mask_b() const;
        bool is_color_mask_a() const;
        
        bool get_is_batching() const;
        
        ui32 get_z_order() const;
        
        const std::string& get_guid() const;
        
        shader_shared_ptr get_shader() const;
        texture_shared_ptr get_texture(e_shader_sampler sampler) const;
        e_shader_sampler get_sampler_index(const texture_shared_ptr& texture) const;
        
        void set_culling(bool value);
        void set_culling_mode(ui32 value);
        
        void set_blending(bool value, ui32 attachment_index = 0);
        void set_blending_function_source(ui32 value, ui32 attachment_index = 0);
        void set_blending_function_destination(ui32 value, ui32 attachment_index = 0);
        void set_blending_equation(ui32 value, ui32 attachment_index = 0);
        
        void set_stencil_test(bool value);
        void set_stencil_function(ui32 value);
        void set_stencil_mask_read(i32 value);
        void set_stencil_mask_write(i32 value);
        void set_stencil_ref_value(i32 value);
        
        void set_back_stencil_op_1(i32 value);
        void set_back_stencil_op_2(i32 value);
        void set_back_stencil_op_3(i32 value);
        void set_front_stencil_op_1(i32 value);
        void set_front_stencil_op_2(i32 value);
        void set_front_stencil_op_3(i32 value);
        
        void set_depth_test(bool value);
        void set_depth_mask(bool value);
        
        void set_color_mask_r(bool value);
        void set_color_mask_g(bool value);
        void set_color_mask_b(bool value);
        void set_color_mask_a(bool value);
        
        void set_is_batching(bool value);
        
        void set_z_order(ui32 z_order);
        
        void set_shader(const shader_shared_ptr& shader);
        void set_texture(const texture_shared_ptr& texture, e_shader_sampler sampler);
        
        void set_custom_shader_uniform(const glm::mat4& matrix, const std::string& uniform);
        void set_custom_shader_uniform(glm::mat4* matrices, i32 size, const std::string& uniform);
        void set_custom_shader_uniform(const glm::mat3& matrix, const std::string& uniform);
        void set_custom_shader_uniform(glm::mat3* matrices, i32 size, const std::string& uniform);
        void set_custom_shader_uniform(const glm::vec4& vector, const std::string& uniform);
        void set_custom_shader_uniform(glm::vec4* vectors, i32 size, const std::string& uniform);
        void set_custom_shader_uniform(const glm::vec3& vector, const std::string& uniform);
        void set_custom_shader_uniform(glm::vec3* vectors, i32 size, const std::string& uniform);
        void set_custom_shader_uniform(const glm::vec2& vector, const std::string& uniform);
        void set_custom_shader_uniform(glm::vec2* vectors, i32 size, const std::string& uniform);
        void set_custom_shader_uniform(f32 value, const std::string& uniform);
        void set_custom_shader_uniform(f32* values, i32 size, const std::string& uniform);
        void set_custom_shader_uniform(i32 value, const std::string& uniform);
        void set_custom_shader_uniform(i32* values, i32 size, const std::string& uniform);
        void set_custom_shader_uniforms(const std::unordered_map<std::string, std::shared_ptr<shader_uniform>>& uniforms);
        
        const std::map<std::string, std::shared_ptr<shader_uniform>>& get_custom_uniforms() const;
        
#if USED_GRAPHICS_API == VULKAN_API

		void bind(const VkPipelineVertexInputStateCreateInfo& vertex_input_state);

#elif USED_GRAPHICS_API == METAL_API
        
        void bind(const mtl_vertex_descriptor_shared_ptr& vertex_descriptor);
        
#else

        void bind();

#endif

        void unbind();
        
#if USED_GRAPHICS_API == METAL_API
        
        mtl_render_encoder_shared_ptr get_render_encoder() const;
        mtl_buffer_shared_ptr get_mvp_uniforms_buffer() const;
        mtl_buffer_shared_ptr get_custom_uniform_buffer(ui32 size);
        
#endif
        
    };
};

#endif
