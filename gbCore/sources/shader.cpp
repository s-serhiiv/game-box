//
//  shader.cpp
//  gbCore
//
//  Created by Sergey Sergeev on 8/12/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#include "shader.h"
#include "shader_compiler_glsl.h"
#include "resource_status.h"
#include "texture.h"
#include "vk_device.h"
#include "vk_initializers.h"
#include "vk_utils.h"
#include "vk_swap_chain.h"

namespace gb
{
    static ui32 g_shader_id = 0;

#if USED_GRAPHICS_API == VULKAN_API

	static shader* g_vk_shader = nullptr;

#endif
    
    extern const struct attribute_names
    {
        std::string m_position;
        std::string m_texcoord;
        std::string m_color;
        std::string m_normal;
        std::string m_tangent;
        std::string m_extra;
        
    } attributes_names;
    
    extern const struct uniform_names
    {
        std::string m_mat_m;
        std::string m_mat_p;
        std::string m_mat_v;
		std::string m_mat_n;
        
    } uniform_names;
    
    extern const struct sampler_names
    {
        std::string m_sampler_01;
        std::string m_sampler_02;
        std::string m_sampler_03;
        std::string m_sampler_04;
        std::string m_sampler_05;
        std::string m_sampler_06;
        std::string m_sampler_07;
        std::string m_sampler_08;
        
    } sampler_names;
    
    const struct attribute_names attribute_names =
    {
        "a_position",
        "a_texcoord",
        "a_color",
        "a_normal",
        "a_tangent",
        "a_extra"
    };
    
    const struct uniform_names uniform_names =
    {
        "u_mat_m",
        "u_mat_p",
		"u_mat_v",
		"u_mat_n"
    };
    
    const struct sampler_names sampler_names =
    {
        "sampler_01",
        "sampler_02",
        "sampler_03",
        "sampler_04",
        "sampler_05",
        "sampler_06",
        "sampler_07",
        "sampler_08"
    };
    
    shader_uniform::shader_uniform(e_uniform_type type, ui32 size) :
    m_type(type),
    m_mat4_value(0.f),
    m_mat4_array(nullptr),
    m_mat3_value(0.f),
    m_mat3_array(nullptr),
    m_vec4_value(0.f),
    m_vec4_array(nullptr),
    m_vec3_value(0.f),
    m_vec3_array(nullptr),
    m_vec2_value(0.f),
    m_vec2_array(nullptr),
    m_f32_value(0.f),
    m_f32_array(nullptr),
    m_i32_value(0),
    m_i32_array(nullptr),
    m_sampler_value(e_shader_sampler_01),
    m_texture_value(nullptr),
    m_array_size(size)
    {

    }
    
    shader_uniform::~shader_uniform()
    {
        
    }
    
    e_uniform_type shader_uniform::get_type() const
    {
        return m_type;
    }
    
    void shader_uniform::set(const glm::mat4& matrix)
    {
        assert(m_type == e_uniform_type_mat4);
        m_mat4_value = matrix;

    }
    
    void shader_uniform::set(glm::mat4 *matrices, i32 size)
    {
        assert(m_type == e_uniform_type_mat4_array);
        m_mat4_array = matrices;
		assert(m_array_size == size);
        m_array_size = size;

    }
    
    void shader_uniform::set(const glm::mat3& matrix)
    {
        assert(m_type == e_uniform_type_mat3);
        m_mat3_value = matrix;

    }
    
    void shader_uniform::set(glm::mat3 *matrices, i32 size)
    {
        assert(m_type == e_uniform_type_mat3_array);
        m_mat3_array = matrices;
		assert(m_array_size == size);
        m_array_size = size;

    }
    
    void shader_uniform::set(const glm::vec4& vector)
    {
        assert(m_type == e_uniform_type_vec4);
        m_vec4_value = vector;

    }
    
    void shader_uniform::set(glm::vec4 *vectors, i32 size)
    {
        assert(m_type = e_uniform_type_vec4_array);
        m_vec4_array = vectors;
		assert(m_array_size == size);
        m_array_size = size;

    }
    
    void shader_uniform::set(const glm::vec3& vector)
    {
        assert(m_type == e_uniform_type_vec3);
        m_vec3_value = vector;

    }
    
    void shader_uniform::set(glm::vec3 *vectors, i32 size)
    {
        assert(m_type = e_uniform_type_vec3_array);
        m_vec3_array = vectors;
		assert(m_array_size == size);
        m_array_size = size;

    }
    
    void shader_uniform::set(const glm::vec2& vector)
    {
        assert(m_type == e_uniform_type_vec2);
        m_vec2_value = vector;

    }
    
    void shader_uniform::set(glm::vec2 *vectors, i32 size)
    {
        assert(m_type = e_uniform_type_vec2_array);
        m_vec2_array = vectors;
		assert(m_array_size == size);
        m_array_size = size;

    }
    
    void shader_uniform::set(f32 value)
    {
        assert(m_type == e_uniform_type_f32);
        m_f32_value = value;

    }
    
    void shader_uniform::set(f32 *values, i32 size)
    {
        assert(m_type = e_uniform_type_f32_array);
        m_f32_array = values;
		assert(m_array_size == size);
        m_array_size = size;

    }
    
    void shader_uniform::set(i32 value)
    {
        assert(m_type == e_uniform_type_i32);
        m_i32_value = value;

    }
    
    void shader_uniform::set(i32* values, i32 size)
    {
        assert(m_type = e_uniform_type_f32_array);
        m_i32_array = values;
		assert(m_array_size == size);
        m_array_size = size;

    }
    
    void shader_uniform::set(const std::shared_ptr<texture> &texture, gb::e_shader_sampler sampler)
    {
        assert(m_type == e_uniform_type_sampler);
        m_texture_value = texture;
        m_sampler_value = sampler;
    }
    
    const glm::mat4& shader_uniform::get_mat4() const
    {
        assert(m_type == e_uniform_type_mat4);
        return m_mat4_value;
    }
    
    const glm::mat4* shader_uniform::get_mat4_array() const
    {
        assert(m_type == e_uniform_type_mat4_array);
        return m_mat4_array;
    }
    
    const glm::mat3& shader_uniform::get_mat3() const
    {
        assert(m_type == e_uniform_type_mat3);
        return m_mat3_value;
    }
    
    const glm::mat3* shader_uniform::get_mat3_array() const
    {
        assert(m_type == e_uniform_type_mat3_array);
        return m_mat3_array;
    }
    
    const glm::vec4& shader_uniform::get_vec4() const
    {
        assert(m_type == e_uniform_type_vec4);
        return m_vec4_value;
    }
    
    const glm::vec4* shader_uniform::get_vec4_array() const
    {
        assert(m_type == e_uniform_type_vec4_array);
        return m_vec4_array;
    }
    
    const glm::vec3& shader_uniform::get_vec3() const
    {
        assert(m_type == e_uniform_type_vec3);
        return m_vec3_value;
    }
    
    const glm::vec3* shader_uniform::get_vec3_array() const
    {
        assert(m_type == e_uniform_type_vec3_array);
        return m_vec3_array;
    }
    
    const glm::vec2& shader_uniform::get_vec2() const
    {
        assert(m_type == e_uniform_type_vec2);
        return m_vec2_value;
    }
    
    const glm::vec2* shader_uniform::get_vec2_array() const
    {
        assert(m_type == e_uniform_type_vec2_array);
        return m_vec2_array;
    }
    
    f32 shader_uniform::get_f32() const
    {
        assert(m_type == e_uniform_type_f32);
        return m_f32_value;
    }
    
    f32* shader_uniform::get_f32_array() const
    {
        assert(m_type == e_uniform_type_f32_array);
        return m_f32_array;
    }
    
    i32 shader_uniform::get_i32() const
    {
        assert(m_type == e_uniform_type_i32);
        return m_i32_value;
    }
    
    i32* shader_uniform::get_i32_array() const
    {
        assert(m_type == e_uniform_type_i32_array);
        return m_i32_array;
    }
    
    e_shader_sampler shader_uniform::get_sampler() const
    {
        assert(m_type == e_uniform_type_sampler);
        return m_sampler_value;
    }
    
    std::shared_ptr<texture> shader_uniform::get_texture() const
    {
        assert(m_type == e_uniform_type_sampler);
        return m_texture_value;
    }
    
    ui32 shader_uniform::get_array_size() const
    {
        return m_array_size;
    }

    shader_transfering_data::shader_transfering_data() :
    m_shader_id(0),
    m_vs_source_code(""),
    m_fs_source_code("")
    {
        m_type = e_resource_transfering_data_type_shader;
    }
    
    shader::shader(const std::string& guid) :
    gb::resource(e_resource_type_shader, guid)
    {
        m_attributes[e_shader_attribute_position] = -1;
        m_attributes[e_shader_attribute_texcoord] = -1;
        m_attributes[e_shader_attribute_color] = -1;
        m_attributes[e_shader_attribute_normal] = -1;
        m_attributes[e_shader_attribute_tangent] = -1;
        m_attributes[e_shader_attribute_extra] = -1;
    }
    
    shader_shared_ptr shader::construct(const std::string& guid,
                                        const std::string& vs_source_code,
                                        const std::string& fs_source_code)
    {
        shader_shared_ptr shader = std::make_shared<gb::shader>(guid);

#if USED_GRAPHICS_API == VULKAN_API

		shader->m_vs_source_code = vs_source_code;
		shader->m_fs_source_code = fs_source_code;

#endif
        
        std::string out_message = "";
        bool out_success = false;
		
#if USED_GRAPHICS_API == VULKAN_API

		VkPipelineShaderStageCreateInfo vs_handle;
		VkPipelineShaderStageCreateInfo fs_handle;

#else

		ui32 vs_handle = 0;
		ui32 fs_handle = 0;

#endif
        
        vs_handle = shader_compiler_glsl::compile(vs_source_code, gl::constant::vertex_shader, &out_message, &out_success);

        if(!out_success)
        {
            std::cout<<out_message<<std::endl;
            return nullptr;
        }

        fs_handle = shader_compiler_glsl::compile(fs_source_code, gl::constant::fragment_shader, &out_message, &out_success);

        if(!out_success)
        {
            std::cout<<out_message<<std::endl;
            return nullptr;
        }
        
		ui32 shader_id = 0;

#if USED_GRAPHICS_API != VULKAN_API

        shader_id = shader_compiler_glsl::link(vs_handle, fs_handle, &out_message, &out_success);
        if(!out_success)
        {
            std::cout<<out_message<<std::endl;
            return nullptr;
        }

#endif
        
        shader->m_shader_id = shader_id;

#if USED_GRAPHICS_API == VULKAN_API

		shader->m_vs_shader_stage = vs_handle;
		shader->m_fs_shader_stage = fs_handle;

#endif

        shader->setup_uniforms();
        
        shader->m_status |= e_resource_status_loaded;
        shader->m_status |= e_resource_status_commited;
        return shader;
    }
    
    shader::~shader()
    {
#if USED_GRAPHICS_API == VULKAN_API

		const auto device = vk_device::get_instance()->get_logical_device();
		if(m_pipeline_layout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(device, m_pipeline_layout, nullptr);
		}
		for (auto descriptor_pool : m_vk_frame_descriptor_pools)
		{
			if (descriptor_pool != VK_NULL_HANDLE)
			{
				vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
			}
		}
		if(m_vk_descriptor_set_layout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(device, m_vk_descriptor_set_layout, nullptr);
		}
		if(m_vs_shader_stage.module != VK_NULL_HANDLE)
		{
			vkDestroyShaderModule(device, m_vs_shader_stage.module, nullptr);
		}
		if(m_fs_shader_stage.module != VK_NULL_HANDLE)
		{
			vkDestroyShaderModule(device, m_fs_shader_stage.module, nullptr);
		}

#else

        gl::command::delete_program(m_shader_id);

#endif
    }
    
    void shader::on_transfering_data_serialized(const std::shared_ptr<resource_transfering_data> &data)
    {
        assert(data != nullptr);
        switch(data->get_type())
        {
            case e_resource_transfering_data_type_shader:
            {
                m_status |= e_resource_status_loaded;
            }
                break;
            default:
            {
                assert(false);
            }
                break;
        }
    }
    
    void shader::on_transfering_data_commited(const std::shared_ptr<resource_transfering_data> &data)
    {
        assert(data != nullptr);
        switch(data->get_type())
        {
            case e_resource_transfering_data_type_shader:
            {
                m_shader_id = std::static_pointer_cast<shader_transfering_data>(data)->m_shader_id;

#if USED_GRAPHICS_API == VULKAN_API

				m_vs_shader_stage = std::static_pointer_cast<shader_transfering_data>(data)->m_vs_shader_stage;
				m_fs_shader_stage = std::static_pointer_cast<shader_transfering_data>(data)->m_fs_shader_stage;
				m_vs_source_code = std::static_pointer_cast<shader_transfering_data>(data)->m_vs_source_code;
				m_fs_source_code = std::static_pointer_cast<shader_transfering_data>(data)->m_fs_source_code;

#endif

                shader::setup_uniforms();
                m_status |= e_resource_status_commited;
            }
                break;
            default:
            {
                assert(false);
            }
                break;
        }
    }
    
    void shader::setup_uniforms()
    {
		m_cached_uniform.resize(e_shader_uniform_max + e_shader_sampler_max, nullptr);

        m_uniforms[e_shader_uniform_mat_m] = gl::command::get_uniform_location(m_shader_id, uniform_names.m_mat_m.c_str());
        m_uniforms[e_shader_uniform_mat_p] = gl::command::get_uniform_location(m_shader_id, uniform_names.m_mat_p.c_str());
        m_uniforms[e_shader_uniform_mat_v] = gl::command::get_uniform_location(m_shader_id, uniform_names.m_mat_v.c_str());
		m_uniforms[e_shader_uniform_mat_n] = gl::command::get_uniform_location(m_shader_id, uniform_names.m_mat_n.c_str());
        
        m_samplers[e_shader_sampler_01] = gl::command::get_uniform_location(m_shader_id, sampler_names.m_sampler_01.c_str());
        m_samplers[e_shader_sampler_02] = gl::command::get_uniform_location(m_shader_id, sampler_names.m_sampler_02.c_str());
        m_samplers[e_shader_sampler_03] = gl::command::get_uniform_location(m_shader_id, sampler_names.m_sampler_03.c_str());
        m_samplers[e_shader_sampler_04] = gl::command::get_uniform_location(m_shader_id, sampler_names.m_sampler_04.c_str());
        m_samplers[e_shader_sampler_05] = gl::command::get_uniform_location(m_shader_id, sampler_names.m_sampler_05.c_str());
        m_samplers[e_shader_sampler_06] = gl::command::get_uniform_location(m_shader_id, sampler_names.m_sampler_06.c_str());
        m_samplers[e_shader_sampler_07] = gl::command::get_uniform_location(m_shader_id, sampler_names.m_sampler_07.c_str());
        m_samplers[e_shader_sampler_08] = gl::command::get_uniform_location(m_shader_id, sampler_names.m_sampler_08.c_str());

#if USED_GRAPHICS_API == VULKAN_API

		VkDescriptorSetLayoutBinding mat_m_binding = vk_initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
		VkDescriptorSetLayoutBinding mat_p_binding = vk_initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, 1);
		VkDescriptorSetLayoutBinding mat_v_binding = vk_initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 2, 1);
		VkDescriptorSetLayoutBinding mat_n_binding = vk_initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 13, 1);

		VkDescriptorSetLayoutBinding sampler_01_binding = vk_initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3, 1);
		VkDescriptorSetLayoutBinding sampler_02_binding = vk_initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4, 1);
		VkDescriptorSetLayoutBinding sampler_03_binding = vk_initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 5, 1);
		VkDescriptorSetLayoutBinding sampler_04_binding = vk_initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 6, 1);
		VkDescriptorSetLayoutBinding sampler_05_binding = vk_initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 7, 1);
		VkDescriptorSetLayoutBinding sampler_06_binding = vk_initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 8, 1);
		VkDescriptorSetLayoutBinding sampler_07_binding = vk_initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 9, 1);
		VkDescriptorSetLayoutBinding sampler_08_binding = vk_initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 10, 1);
		VkDescriptorSetLayoutBinding vs_custom_uniforms_binding = vk_initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 11, 1);
		VkDescriptorSetLayoutBinding fs_custom_uniforms_binding = vk_initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 12, 1);
        
		std::vector<VkDescriptorSetLayoutBinding> bindings = { 
			mat_m_binding,
			mat_p_binding,
			mat_v_binding,
			sampler_01_binding, 
			sampler_02_binding,
			sampler_03_binding, 
			sampler_04_binding, 
			sampler_05_binding,
			sampler_06_binding, 
			sampler_07_binding, 
			sampler_08_binding,
			vs_custom_uniforms_binding,
			fs_custom_uniforms_binding,
			mat_n_binding
		};

		VkDescriptorSetLayoutCreateInfo layout_create_info = vk_initializers::descriptor_set_layout_create_info(bindings);
		VK_CHECK(vkCreateDescriptorSetLayout(vk_device::get_instance()->get_logical_device(), &layout_create_info, nullptr, &m_vk_descriptor_set_layout));

		VkDescriptorSetLayout set_layouts[] = { m_vk_descriptor_set_layout };
		VkPipelineLayoutCreateInfo pipeline_layout_info = vk_initializers::pipeline_layout_create_info(set_layouts, 1);

		VK_CHECK(vkCreatePipelineLayout(vk_device::get_instance()->get_logical_device(), &pipeline_layout_info, nullptr, &m_pipeline_layout));

		m_cached_uniform[e_shader_uniform_mat_m] = std::make_shared<shader_uniform>(e_uniform_type_mat4);
		m_cached_uniform[e_shader_uniform_mat_p] = std::make_shared<shader_uniform>(e_uniform_type_mat4);
		m_cached_uniform[e_shader_uniform_mat_v] = std::make_shared<shader_uniform>(e_uniform_type_mat4);
		m_cached_uniform[e_shader_uniform_mat_n] = std::make_shared<shader_uniform>(e_uniform_type_mat4);

		shader_compiler_glsl::convert_to_vulkan_source(m_vs_source_code, 11, &m_vk_vs_custom_uniforms);
		shader_compiler_glsl::convert_to_vulkan_source(m_fs_source_code, 12, &m_vk_fs_custom_uniforms);
		const ui32 vs_uniforms_size = m_vk_vs_custom_uniforms.empty() ? 16 : m_vk_vs_custom_uniforms.back().m_offset + m_vk_vs_custom_uniforms.back().m_stride * m_vk_vs_custom_uniforms.back().m_array_size;
		const ui32 fs_uniforms_size = m_vk_fs_custom_uniforms.empty() ? 16 : m_vk_fs_custom_uniforms.back().m_offset + m_vk_fs_custom_uniforms.back().m_stride * m_vk_fs_custom_uniforms.back().m_array_size;
		m_vk_vs_custom_uniforms_data.resize(vs_uniforms_size, 0);
		m_vk_fs_custom_uniforms_data.resize(fs_uniforms_size, 0);
		ui8 fallback_pixel[] = { 255, 255, 255, 255 };
		m_vk_fallback_texture = texture::construct(get_guid() + ".fallback", 1, 1, gl::constant::rgba_t, fallback_pixel);
		const ui32 images_count = vk_swap_chain::get_instance()->get_images_count();
		m_vk_frame_descriptor_pools.resize(images_count, VK_NULL_HANDLE);
		m_vk_frame_numbers.resize(images_count, 0);
		m_vk_frame_uniform_buffers.resize(images_count);
		m_vk_frame_uniform_buffer_offsets.resize(images_count, 0);

#endif

        m_attributes.at(e_shader_attribute_position) = gl::command::get_attribute_location(m_shader_id, attribute_names.m_position.c_str());
        m_attributes.at(e_shader_attribute_texcoord) = gl::command::get_attribute_location(m_shader_id, attribute_names.m_texcoord.c_str());
        m_attributes.at(e_shader_attribute_color) = gl::command::get_attribute_location(m_shader_id, attribute_names.m_color.c_str());
        m_attributes.at(e_shader_attribute_normal) = gl::command::get_attribute_location(m_shader_id, attribute_names.m_normal.c_str());
        m_attributes.at(e_shader_attribute_tangent) = gl::command::get_attribute_location(m_shader_id, attribute_names.m_tangent.c_str());
        m_attributes.at(e_shader_attribute_extra) = gl::command::get_attribute_location(m_shader_id, attribute_names.m_extra.c_str());

    }

    const std::array<i32, e_shader_attribute_max>& shader::get_attributes() const
    {
        return m_attributes;
    }
    
    i32 shader::get_custom_uniform(const std::string& uniform)
    {
        i32 handle = -1;
        const auto iterator = m_custom_uniforms.find(uniform);
        if(iterator != m_custom_uniforms.end())
        {
            handle = iterator->second;
        }
        else
        {
            handle = gl::command::get_uniform_location(m_shader_id, uniform.c_str());
            m_custom_uniforms.insert(std::make_pair(uniform, handle));
        }
        return handle;
    }

#if USED_GRAPHICS_API == VULKAN_API

	void shader::set_vk_custom_uniform(const std::string& uniform, const void* data, ui32 elements_count)
	{
		auto apply = [&](const std::vector<shader_custom_uniform_desc>& descriptions, std::vector<ui8>& buffer_data) {
			for(const auto& description : descriptions)
			{
				if(description.m_name == uniform)
				{
					const ui32 count = std::min(elements_count, description.m_array_size);
					if(description.m_type == "mat3")
					{
						const f32* source = static_cast<const f32*>(data);
						for(ui32 element = 0; element < count; ++element)
						{
							for(ui32 column = 0; column < 3; ++column)
							{
								memcpy(buffer_data.data() + description.m_offset + element * description.m_stride + column * 16, source + element * 9 + column * 3, 12);
							}
						}
					}
					else
					{
						ui32 source_stride = description.m_element_size;
						for(ui32 element = 0; element < count; ++element)
						{
							memcpy(buffer_data.data() + description.m_offset + element * description.m_stride, static_cast<const ui8*>(data) + element * source_stride, description.m_element_size);
						}
					}
					break;
				}
			}
		};

		apply(m_vk_vs_custom_uniforms, m_vk_vs_custom_uniforms_data);
		apply(m_vk_fs_custom_uniforms, m_vk_fs_custom_uniforms_data);
	}

#endif
    
    void shader::set_mat3(const glm::mat3 &matrix, e_shader_uniform uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            if(m_cached_uniform[uniform] != nullptr && m_cached_uniform[uniform]->get_mat3() == matrix)
            {
                return;
            }
            else if(m_cached_uniform[uniform] == nullptr)
            {
                m_cached_uniform[uniform] = std::make_shared<shader_uniform>(e_uniform_type_mat3);
            }
            
            i32 handle = m_uniforms[uniform];
            gl::command::get_uniform_matrix_3fv(handle, 1, 0, &matrix[0][0]);
            m_cached_uniform[uniform]->set(matrix);
        }
    }
    
    void shader::set_custom_mat3(const glm::mat3x3 &matrix, const std::string &uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            gl::command::get_uniform_matrix_3fv(shader::get_custom_uniform(uniform), 1, 0, &matrix[0][0]);

#if USED_GRAPHICS_API == VULKAN_API

			set_vk_custom_uniform(uniform, &matrix[0][0]);

#endif
        }
    }
    
    void shader::set_mat4(const glm::mat4x4 &matrix, e_shader_uniform uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            if(m_cached_uniform[uniform] != nullptr && m_cached_uniform[uniform]->get_mat4() == matrix)
            {
                return;
            }
            else if(m_cached_uniform[uniform] == nullptr)
            {
                m_cached_uniform[uniform] = std::make_shared<shader_uniform>(e_uniform_type_mat4);
            }
            
            i32 handle = m_uniforms[uniform];
            gl::command::get_uniform_matrix_4fv(handle, 1, 0, &matrix[0][0]);
            m_cached_uniform[uniform]->set(matrix);
        }
    }
    
    void shader::set_custom_mat4(const glm::mat4x4 &matrix, const std::string &uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            gl::command::get_uniform_matrix_4fv(shader::get_custom_uniform(uniform), 1, 0, &matrix[0][0]);

#if USED_GRAPHICS_API == VULKAN_API

			set_vk_custom_uniform(uniform, &matrix[0][0]);

#endif
        }
    }
    
    void shader::set_mat4_array(const glm::mat4x4* matrix, ui32 size, e_shader_uniform uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            i32 handle = m_uniforms[uniform];
            gl::command::get_uniform_matrix_4fv(handle, size, 0, &matrix[0][0][0]);
        }
    }
    
    void shader::set_custom_mat4_array(const glm::mat4x4* matrix, ui32 size, const std::string& uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            gl::command::get_uniform_matrix_4fv(shader::get_custom_uniform(uniform), size, 0, &matrix[0][0][0]);

#if USED_GRAPHICS_API == VULKAN_API

			set_vk_custom_uniform(uniform, &matrix[0][0][0], size);

#endif
        }
    }
    
    void shader::set_vec2(const glm::vec2 &vector, e_shader_uniform uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            if(m_cached_uniform[uniform] != nullptr && m_cached_uniform[uniform]->get_vec2() == vector)
            {
                return;
            }
            else if(m_cached_uniform[uniform] == nullptr)
            {
                m_cached_uniform[uniform] = std::make_shared<shader_uniform>(e_uniform_type_vec2);
            }
            
            i32 handle = m_uniforms[uniform];
            gl::command::get_uniform_vector_2fv(handle, 1, &vector[0]);
            m_cached_uniform[uniform]->set(vector);
        }
    }
    
    void shader::set_custom_vec2(const glm::vec2 &vector, const std::string &uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            gl::command::get_uniform_vector_2fv(shader::get_custom_uniform(uniform), 1, &vector[0]);

#if USED_GRAPHICS_API == VULKAN_API

			set_vk_custom_uniform(uniform, &vector[0]);

#endif
        }
    }
    
    void shader::set_custom_vec2_array(const glm::vec2* vectors, ui32 size, const std::string& uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            gl::command::get_uniform_vector_2fv(shader::get_custom_uniform(uniform), size, &vectors[0][0]);

#if USED_GRAPHICS_API == VULKAN_API

			set_vk_custom_uniform(uniform, &vectors[0][0], size);

#endif
        }
    }
    
    void shader::set_vec3(const glm::vec3 &vector, e_shader_uniform uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            if(m_cached_uniform[uniform] != nullptr && m_cached_uniform[uniform]->get_vec3() == vector)
            {
                return;
            }
            else if(m_cached_uniform[uniform] == nullptr)
            {
                m_cached_uniform[uniform] = std::make_shared<shader_uniform>(e_uniform_type_vec3);
            }
            
            i32 handle = m_uniforms[uniform];
            gl::command::get_uniform_vector_3fv(handle, 1, &vector[0]);
            m_cached_uniform[uniform]->set(vector);
        }
    }
    
    void shader::set_custom_vec3(const glm::vec3 &vector, const std::string &uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            gl::command::get_uniform_vector_3fv(shader::get_custom_uniform(uniform), 1, &vector[0]);

#if USED_GRAPHICS_API == VULKAN_API

			set_vk_custom_uniform(uniform, &vector[0]);

#endif
        }
    }
    
    void shader::set_custom_vec3_array(const glm::vec3* vectors, ui32 size, const std::string& uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            gl::command::get_uniform_vector_3fv(shader::get_custom_uniform(uniform), size, &vectors[0][0]);

#if USED_GRAPHICS_API == VULKAN_API

			set_vk_custom_uniform(uniform, &vectors[0][0], size);

#endif
        }
    }
    
    void shader::set_vec4(const glm::vec4 &vector, e_shader_uniform uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            if(m_cached_uniform[uniform] != nullptr && m_cached_uniform[uniform]->get_vec4() == vector)
            {
                return;
            }
            else if(m_cached_uniform[uniform] == nullptr)
            {
                m_cached_uniform[uniform] = std::make_shared<shader_uniform>(e_uniform_type_vec4);
            }
            
            i32 handle = m_uniforms[uniform];
            gl::command::get_uniform_vector_4fv(handle, 1, &vector[0]);
            m_cached_uniform[uniform]->set(vector);
        }
    }
    
    void shader::set_custom_vec4(const glm::vec4 &vector, const std::string &uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            gl::command::get_uniform_vector_4fv(shader::get_custom_uniform(uniform), 1, &vector[0]);

#if USED_GRAPHICS_API == VULKAN_API

			set_vk_custom_uniform(uniform, &vector[0]);

#endif
        }
    }
    
    void shader::set_custom_vec4_array(const glm::vec4* vectors, ui32 size, const std::string& uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            gl::command::get_uniform_vector_4fv(shader::get_custom_uniform(uniform), size, &vectors[0][0]);

#if USED_GRAPHICS_API == VULKAN_API

			set_vk_custom_uniform(uniform, &vectors[0][0], size);

#endif
        }
    }
    
    void shader::set_f32(f32 value, e_shader_uniform uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            if(m_cached_uniform[uniform] != nullptr && m_cached_uniform[uniform]->get_f32() == value)
            {
                return;
            }
            else if(m_cached_uniform[uniform] == nullptr)
            {
                m_cached_uniform[uniform] = std::make_shared<shader_uniform>(e_uniform_type_f32);
            }
            
            i32 handle = m_uniforms[uniform];
            gl::command::get_uniform_1f(handle, value);
            m_cached_uniform[uniform]->set(value);
        }
    }
    
    void shader::set_custom_f32(f32 value, const std::string &uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            gl::command::get_uniform_1f(shader::get_custom_uniform(uniform), value);

#if USED_GRAPHICS_API == VULKAN_API

			set_vk_custom_uniform(uniform, &value);

#endif
        }
    }
    
    void shader::set_i32(i32 value, e_shader_uniform uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            if(m_cached_uniform[uniform] != nullptr && m_cached_uniform[uniform]->get_i32() == value)
            {
                return;
            }
            else if(m_cached_uniform[uniform] == nullptr)
            {
                m_cached_uniform[uniform] = std::make_shared<shader_uniform>(e_uniform_type_i32);
            }
            
            i32 handle = m_uniforms[uniform];
            gl::command::get_uniform_1i(handle, value);
            m_cached_uniform[uniform]->set(value);
        }
    }
    
    void shader::set_custom_i32(i32 value, const std::string &uniform)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            gl::command::get_uniform_1i(shader::get_custom_uniform(uniform), value);

#if USED_GRAPHICS_API == VULKAN_API

			set_vk_custom_uniform(uniform, &value);

#endif
        }
    }
    
    void shader::set_texture(const std::shared_ptr<texture> &texture, gb::e_shader_sampler sampler)
    {
        if(resource::is_loaded() && resource::is_commited())
        {
            assert(sampler < e_shader_sampler_max);
            gl::command::set_active_texture(gl::constant::texture_0 + sampler);
            texture->bind();
            gl::command::get_uniform_1i(m_samplers[sampler], sampler);

#if USED_GRAPHICS_API == VULKAN_API

			const ui32 uniform_index = e_shader_uniform_max + sampler;
			if (!m_cached_uniform[uniform_index])
			{
				m_cached_uniform[uniform_index] = std::make_shared<shader_uniform>(e_uniform_type_sampler);
			}
			m_cached_uniform[uniform_index]->set(texture, sampler);

#endif
        }
    }

	void shader::bind() const
    {
        if(resource::is_loaded() && resource::is_commited() && g_shader_id != m_shader_id)
        {
            g_shader_id = m_shader_id;
            gl::command::use_program(m_shader_id);
        }

#if USED_GRAPHICS_API == VULKAN_API

		g_vk_shader = const_cast<shader*>(this);

#endif
    }
    
    void shader::unbind() const
    {

#if USED_GRAPHICS_API == VULKAN_API

		if (g_vk_shader == this)
		{
			g_vk_shader = nullptr;
		}

#endif
    }
    
    i32 shader::get_custom_attribute(const std::string& attribute_name)
    {
        i32 attribute = gl::command::get_attribute_location(m_shader_id, attribute_name.c_str());
        if(attribute != -1)
        {
            m_custom_attributes.insert(std::make_pair(attribute_name, attribute));
        }
        return attribute;
    }
    
    const std::unordered_map<std::string, i32>& shader::get_custom_attributes() const
    {
        return m_custom_attributes;
    }
    
    bool shader::is_custom_attributes_exist() const
    {
        return m_custom_attributes.size() != 0;
    }
    
    shader_mvp_uniforms shader::get_mvp_uniforms()
    {
        glm::mat4 mat_m = m_cached_uniform[e_shader_uniform_mat_m]->get_mat4();
        m_mvp_uniforms.m_mat_m = mat_m;
        glm::mat4 mat_v = m_cached_uniform[e_shader_uniform_mat_v]->get_mat4();
        m_mvp_uniforms.m_mat_v = mat_v;
        glm::mat4 mat_p = m_cached_uniform[e_shader_uniform_mat_p]->get_mat4();
        m_mvp_uniforms.m_mat_p = mat_p;
        glm::mat4 mat_n = m_cached_uniform[e_shader_uniform_mat_n]->get_mat4();
        m_mvp_uniforms.m_mat_n = mat_n;
        
        return m_mvp_uniforms;
    }

#if USED_GRAPHICS_API == VULKAN_API

	VkPipelineShaderStageCreateInfo shader::get_vs_shader_stage() const
	{
		return m_vs_shader_stage;
	}

	VkPipelineShaderStageCreateInfo shader::get_fs_shader_stage() const
	{
		return m_fs_shader_stage;
	}

	std::vector<VkPipelineShaderStageCreateInfo> shader::get_shader_stages() const
	{
		return { m_vs_shader_stage , m_fs_shader_stage };
	}

	VkPipelineLayout shader::get_pipeline_layout() const
	{
		return m_pipeline_layout;
	}

	VkDescriptorSet shader::construct_descriptor_set()
	{
		const auto device = vk_device::get_instance();
		const VkDevice logical_device = device->get_logical_device();
		const ui32 image_index = device->get_current_image_index();
		const ui64 frame_number = device->get_frame_number(image_index);
		if (m_vk_frame_descriptor_pools[image_index] == VK_NULL_HANDLE)
		{
			const ui32 descriptor_sets_count = 1024;
			std::vector<VkDescriptorPoolSize> pool_sizes = {
				vk_initializers::descriptor_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 6 * descriptor_sets_count),
				vk_initializers::descriptor_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, e_shader_sampler_max * descriptor_sets_count)
			};
			VkDescriptorPoolCreateInfo pool_create_info = vk_initializers::descriptor_pool_create_info(pool_sizes, descriptor_sets_count);
			VK_CHECK(vkCreateDescriptorPool(logical_device, &pool_create_info, nullptr, &m_vk_frame_descriptor_pools[image_index]));
		}
		if (m_vk_frame_numbers[image_index] != frame_number)
		{
			VK_CHECK(vkResetDescriptorPool(logical_device, m_vk_frame_descriptor_pools[image_index], 0));
			m_vk_frame_uniform_buffer_offsets[image_index] = 0;
			m_vk_frame_numbers[image_index] = frame_number;
		}
		if (!m_vk_frame_uniform_buffers[image_index])
		{
			const VkDeviceSize uniform_buffer_size = 2 * 1024 * 1024;
			m_vk_frame_uniform_buffers[image_index] = std::make_shared<vk_buffer>();
			VK_CHECK(vk_utils::create_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
										 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
										 m_vk_frame_uniform_buffers[image_index], uniform_buffer_size));
			VK_CHECK(m_vk_frame_uniform_buffers[image_index]->map());
		}

		VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
		VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {};
		descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptor_set_alloc_info.descriptorPool = m_vk_frame_descriptor_pools[image_index];
		descriptor_set_alloc_info.descriptorSetCount = 1;
		descriptor_set_alloc_info.pSetLayouts = &m_vk_descriptor_set_layout;
		VK_CHECK(vkAllocateDescriptorSets(logical_device, &descriptor_set_alloc_info, &descriptor_set));

		VkPhysicalDeviceProperties device_properties = {};
		vkGetPhysicalDeviceProperties(device->get_physical_device(), &device_properties);
		const VkDeviceSize uniform_buffer_alignment = device_properties.limits.minUniformBufferOffsetAlignment;
		auto construct_uniform_buffer_info = [&](const void* data, VkDeviceSize size) {
			auto& offset = m_vk_frame_uniform_buffer_offsets[image_index];
			offset = (offset + uniform_buffer_alignment - 1) & ~(uniform_buffer_alignment - 1);
			const auto buffer = m_vk_frame_uniform_buffers[image_index];
			assert(offset + size <= buffer->get_size());
			memcpy(static_cast<ui8*>(buffer->get_mapped_data()) + offset, data, size);
			VkDescriptorBufferInfo result = {};
			result.buffer = buffer->get_handler();
			result.offset = offset;
			result.range = size;
			offset += size;
			return result;
		};

		glm::mat4 mat_m = m_cached_uniform[e_shader_uniform_mat_m]->get_mat4();
		glm::mat4 mat_p = m_cached_uniform[e_shader_uniform_mat_p]->get_mat4();
		glm::mat4 mat_v = m_cached_uniform[e_shader_uniform_mat_v]->get_mat4();
		glm::mat4 mat_n = m_cached_uniform[e_shader_uniform_mat_n]->get_mat4();
		std::array<VkDescriptorBufferInfo, 6> buffer_infos = {
			construct_uniform_buffer_info(&mat_m[0][0], sizeof(glm::mat4)),
			construct_uniform_buffer_info(&mat_p[0][0], sizeof(glm::mat4)),
			construct_uniform_buffer_info(&mat_v[0][0], sizeof(glm::mat4)),
			construct_uniform_buffer_info(m_vk_vs_custom_uniforms_data.data(), m_vk_vs_custom_uniforms_data.size()),
			construct_uniform_buffer_info(m_vk_fs_custom_uniforms_data.data(), m_vk_fs_custom_uniforms_data.size()),
			construct_uniform_buffer_info(&mat_n[0][0], sizeof(glm::mat4))
		};
		const ui32 bindings[] = { 0, 1, 2, 11, 12, 13 };
		std::array<VkWriteDescriptorSet, 6> descriptor_writes;
		for (ui32 i = 0; i < descriptor_writes.size(); ++i)
		{
			descriptor_writes[i] = vk_initializers::write_descriptor_set(descriptor_set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bindings[i], &buffer_infos[i]);
		}
		vkUpdateDescriptorSets(logical_device, static_cast<ui32>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);

		for (ui32 sampler = 0; sampler < e_shader_sampler_max; ++sampler)
		{
			texture_shared_ptr texture = m_vk_fallback_texture;
			const ui32 uniform_index = e_shader_uniform_max + sampler;
			if (m_cached_uniform[uniform_index] && m_cached_uniform[uniform_index]->get_texture()->get_vk_image_view() != VK_NULL_HANDLE)
			{
				texture = m_cached_uniform[uniform_index]->get_texture();
			}
			VkDescriptorImageInfo image_info = vk_initializers::descriptor_image_info(texture->get_vk_sampler(), texture->get_vk_image_view(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			VkWriteDescriptorSet descriptor_write = vk_initializers::write_descriptor_set(descriptor_set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 + sampler, &image_info);
			vkUpdateDescriptorSets(logical_device, 1, &descriptor_write, 0, nullptr);
		}
		return descriptor_set;
	}

	void shader::bind_vulkan_descriptor_set()
	{
		assert(g_vk_shader);
		const auto device = vk_device::get_instance();
		const VkDescriptorSet descriptor_set = g_vk_shader->construct_descriptor_set();
		vkCmdBindDescriptorSets(device->get_draw_cmd_buffer(device->get_current_image_index()), VK_PIPELINE_BIND_POINT_GRAPHICS,
							g_vk_shader->get_pipeline_layout(), 0, 1, &descriptor_set, 0, nullptr);
	}

#endif
}
