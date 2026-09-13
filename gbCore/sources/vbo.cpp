//
//  vbo.cpp
//  gbCore
//
//  Created by sergey.sergeev on 8/13/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#include "vbo.h"
#include "vk_device.h"
#include "vk_utils.h"
#include "vk_initializers.h"

#if USED_GRAPHICS_API == METAL_API

#include "mtl_buffer.h"
#include "mtl_vertex_descriptor.h"

#endif

namespace gb
{
	std::unordered_map<std::string, ui32>  vbo::vertex_declaration::m_attributes_locations = {
	{ "m_position", 0 },
	{ "m_texcoord", 1 },
	{ "m_normal", 2 },
	{ "m_tangent", 3 },
	{ "m_color", 4 },
	{ "m_extra", 5 }
	};

    vbo::vertex_declaration::vertex_declaration(ui32 size, vertex_attribute* external_data) :
    m_size(size),
    m_data(nullptr)
    {

#if USED_GRAPHICS_API == VULKAN_API

		m_bindings_description.resize(1);

		VkVertexInputBindingDescription binding_description = {};
		binding_description.binding = 0;
		binding_description.stride = sizeof(vertex_attribute);
		binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		m_bindings_description[0] = binding_description;

#endif

        if(!external_data)
        {
            m_data = new vertex_attribute[m_size];
            memset(m_data, 0x0, sizeof(vertex_attribute) * m_size);
        }
        else
        {
            m_data = external_data;
        }
        m_is_external_data = external_data != nullptr;
    }
    
    vbo::vertex_declaration::~vertex_declaration()
    {
        if(!m_is_external_data)
        {
            delete[] m_data;
        }
    }
    
    vbo::vertex_attribute* vbo::vertex_declaration::get_data() const
    {
        return m_data;
    }
    
    ui32 vbo::vertex_declaration::get_size() const
    {
        return m_size;
    }

#if USED_GRAPHICS_API == VULKAN_API

	VkPipelineVertexInputStateCreateInfo vbo::vertex_declaration::get_vertex_input_state() const
	{
		return m_vertex_input_state;
	}

#endif
    
    
#if USED_GRAPHICS_API == METAL_API
    
    mtl_vertex_descriptor_shared_ptr vbo::vertex_declaration::get_mtl_vertex_descriptor() const
    {
        return m_mtl_vertex_descriptor;
    }
    
#endif
    
    vbo::vertex_declaration_PTC::vertex_declaration_PTC(ui32 size, vertex_attribute* external_data) :
    vertex_declaration(size, external_data)
    {

#if USED_GRAPHICS_API == VULKAN_API

		m_attributes_description.resize(6);

		m_attributes_description[0].binding = 0;
		m_attributes_description[0].location = m_attributes_locations["m_position"];
		m_attributes_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		m_attributes_description[0].offset = offsetof(vertex_attribute_PTC, m_position);

		m_attributes_description[1].binding = 0;
		m_attributes_description[1].location = m_attributes_locations["m_texcoord"];
		m_attributes_description[1].format = VK_FORMAT_R32G32_SFLOAT;
		m_attributes_description[1].offset = offsetof(vertex_attribute_PTC, m_texcoord);

		m_attributes_description[2].binding = 0;
		m_attributes_description[2].location = m_attributes_locations["m_color"];;
		m_attributes_description[2].format = VK_FORMAT_B8G8R8A8_UNORM;
		m_attributes_description[2].offset = offsetof(vertex_attribute_PTC, m_color);

		m_attributes_description[3].binding = 0;
		m_attributes_description[3].location = m_attributes_locations["m_normal"];
		m_attributes_description[3].format = VK_FORMAT_R8G8B8A8_UNORM;
		m_attributes_description[3].offset = offsetof(vertex_attribute_PTC, m_unused);

		m_attributes_description[4].binding = 0;
		m_attributes_description[4].location = m_attributes_locations["m_tangent"];
		m_attributes_description[4].format = VK_FORMAT_R8G8B8A8_UNORM;
		m_attributes_description[4].offset = offsetof(vertex_attribute_PTC, m_unused) + 4;

		m_attributes_description[5].binding = 0;
		m_attributes_description[5].location = m_attributes_locations["m_extra"];
		m_attributes_description[5].format = VK_FORMAT_R8G8B8A8_UNORM;
		m_attributes_description[5].offset = offsetof(vertex_attribute_PTC, m_unused);

		m_vertex_input_state = vk_initializers::pipeline_vertex_input_state_create_info();
		m_vertex_input_state.vertexBindingDescriptionCount = m_bindings_description.size();
		m_vertex_input_state.pVertexBindingDescriptions = m_bindings_description.data();
		m_vertex_input_state.vertexAttributeDescriptionCount = m_attributes_description.size();
		m_vertex_input_state.pVertexAttributeDescriptions = m_attributes_description.data();

#endif
        
#if USED_GRAPHICS_API == METAL_API
        
        m_mtl_vertex_descriptor = mtl_vertex_descriptor::create_PTC_vertex_descriptor();
        
#endif

    }
    
    vbo::vertex_declaration_PTC::~vertex_declaration_PTC()
    {
        
    }
    
    void vbo::vertex_declaration_PTC::bind(const std::array<i32, e_shader_attribute_max>& attributes) const
    {
        if(attributes.at(e_shader_attribute_position) >= 0)
        {
            gl::command::enable_vertex_attribute(attributes.at(e_shader_attribute_position));
            gl::command::bind_vertex_attribute(attributes.at(e_shader_attribute_position), 3, gl::constant::f32_t, gl::constant::no,
                                     sizeof(vertex_attribute_PTC),
                                     (void*)offsetof(vertex_attribute_PTC, m_position));
        }
        if(attributes.at(e_shader_attribute_texcoord) >= 0)
        {
            gl::command::enable_vertex_attribute(attributes.at(e_shader_attribute_texcoord));
            gl::command::bind_vertex_attribute(attributes.at(e_shader_attribute_texcoord), 2, gl::constant::f32_t, gl::constant::no,
                                     sizeof(vertex_attribute_PTC),
                                     (void*)offsetof(vertex_attribute_PTC, m_texcoord));
        }
        if(attributes.at(e_shader_attribute_color) >= 0)
        {
            gl::command::enable_vertex_attribute(attributes.at(e_shader_attribute_color));
            gl::command::bind_vertex_attribute(attributes.at(e_shader_attribute_color), 4, gl::constant::ui8_t, gl::constant::yes,
                                     sizeof(vertex_attribute_PTC),
                                     (void*)offsetof(vertex_attribute_PTC, m_color));
        }
    }
    
    void vbo::vertex_declaration_PTC::unbind(const std::array<i32, e_shader_attribute_max>& attributes) const
    {
        if(attributes.at(e_shader_attribute_position) >= 0)
        {
            gl::command::disable_vertex_attribute(attributes.at(e_shader_attribute_position));
        }
        if(attributes.at(e_shader_attribute_texcoord) >= 0)
        {
            gl::command::disable_vertex_attribute(attributes.at(e_shader_attribute_texcoord));
        }
        if(attributes.at(e_shader_attribute_color) >= 0)
        {
            gl::command::disable_vertex_attribute(attributes.at(e_shader_attribute_color));
        }
    }
    
    vbo::vertex_declaration_PT4B::vertex_declaration_PT4B(ui32 size, vertex_attribute* external_data) :
    vertex_declaration(size, external_data)
    {

#if USED_GRAPHICS_API == VULKAN_API

		m_attributes_description.resize(6);

		m_attributes_description[0].binding = 0;
		m_attributes_description[0].location = m_attributes_locations["m_position"];
		m_attributes_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		m_attributes_description[0].offset = offsetof(vertex_attribute_PT4B, m_position);

		m_attributes_description[1].binding = 0;
		m_attributes_description[1].location = m_attributes_locations["m_texcoord"];
		m_attributes_description[1].format = VK_FORMAT_R16G16_UNORM;
		m_attributes_description[1].offset = offsetof(vertex_attribute_PT4B, m_texcoord);

		m_attributes_description[2].binding = 0;
		m_attributes_description[2].location = m_attributes_locations["m_extra"];
		m_attributes_description[2].format = VK_FORMAT_R8G8B8A8_USCALED;
		m_attributes_description[2].offset = offsetof(vertex_attribute_PT4B, m_bone_ids);

		m_attributes_description[3].binding = 0;
		m_attributes_description[3].location = m_attributes_locations["m_color"];
		m_attributes_description[3].format = VK_FORMAT_R8G8B8A8_USCALED;
		m_attributes_description[3].offset = offsetof(vertex_attribute_PT4B, m_bone_weights);

		m_attributes_description[4].binding = 0;
		m_attributes_description[4].location = m_attributes_locations["m_normal"];
		m_attributes_description[4].format = VK_FORMAT_R8G8B8A8_UNORM;
		m_attributes_description[4].offset = offsetof(vertex_attribute_PT4B, m_unused);

		m_attributes_description[5].binding = 0;
		m_attributes_description[5].location = m_attributes_locations["m_tangent"];
		m_attributes_description[5].format = VK_FORMAT_R8G8B8A8_UNORM;
		m_attributes_description[5].offset = offsetof(vertex_attribute_PT4B, m_unused) + 4;

		m_vertex_input_state = vk_initializers::pipeline_vertex_input_state_create_info();
		m_vertex_input_state.vertexBindingDescriptionCount = m_bindings_description.size();
		m_vertex_input_state.pVertexBindingDescriptions = m_bindings_description.data();
		m_vertex_input_state.vertexAttributeDescriptionCount = m_attributes_description.size();
		m_vertex_input_state.pVertexAttributeDescriptions = m_attributes_description.data();

#endif

    }
    
    vbo::vertex_declaration_PT4B::~vertex_declaration_PT4B()
    {
        
    }
    
    void vbo::vertex_declaration_PT4B::bind(const std::array<i32, e_shader_attribute_max>& attributes) const
    {
        if(attributes.at(e_shader_attribute_position) >= 0)
        {
            gl::command::enable_vertex_attribute(attributes.at(e_shader_attribute_position));
            gl::command::bind_vertex_attribute(attributes.at(e_shader_attribute_position), 3, gl::constant::f32_t, gl::constant::no,
                                     sizeof(vertex_attribute_PT4B),
                                     (void*)offsetof(vertex_attribute_PT4B, m_position));
        }
        if(attributes.at(e_shader_attribute_texcoord) >= 0)
        {
            gl::command::enable_vertex_attribute(attributes.at(e_shader_attribute_texcoord));
            gl::command::bind_vertex_attribute(attributes.at(e_shader_attribute_texcoord), 2, gl::constant::ui16_t, gl::constant::yes,
                                     sizeof(vertex_attribute_PT4B),
                                     (void*)offsetof(vertex_attribute_PT4B, m_texcoord));
        }
        if(attributes.at(e_shader_attribute_color) >= 0)
        {
            gl::command::enable_vertex_attribute(attributes.at(e_shader_attribute_color));
            gl::command::bind_vertex_attribute(attributes.at(e_shader_attribute_color), 4, gl::constant::ui8_t, gl::constant::yes,
                                     sizeof(vertex_attribute_PT4B),
                                     (void*)offsetof(vertex_attribute_PT4B, m_bone_weights));
        }
        if(attributes.at(e_shader_attribute_extra) >= 0)
        {
            gl::command::enable_vertex_attribute(attributes.at(e_shader_attribute_extra));
            gl::command::bind_vertex_attribute(attributes.at(e_shader_attribute_extra), 4, gl::constant::ui8_t, gl::constant::no,
                                     sizeof(vertex_attribute_PT4B),
                                     (void*)offsetof(vertex_attribute_PT4B, m_bone_ids));
        }
    }
    
    void vbo::vertex_declaration_PT4B::unbind(const std::array<i32, e_shader_attribute_max>& attributes) const
    {
        if(attributes.at(e_shader_attribute_position) >= 0)
        {
            gl::command::disable_vertex_attribute(attributes.at(e_shader_attribute_position));
        }
        if(attributes.at(e_shader_attribute_texcoord) >= 0)
        {
            gl::command::disable_vertex_attribute(attributes.at(e_shader_attribute_texcoord));
        }
        if(attributes.at(e_shader_attribute_color) >= 0)
        {
            gl::command::disable_vertex_attribute(attributes.at(e_shader_attribute_color));
        }
        if(attributes.at(e_shader_attribute_extra) >= 0)
        {
            gl::command::disable_vertex_attribute(attributes.at(e_shader_attribute_extra));
        }
    }
    
    vbo::vertex_declaration_PTNTC::vertex_declaration_PTNTC(ui32 size, vertex_attribute* external_data) :
    vertex_declaration(size, external_data)
    {

#if USED_GRAPHICS_API == VULKAN_API

		m_attributes_description.resize(6);

		m_attributes_description[0].binding = 0;
		m_attributes_description[0].location = m_attributes_locations["m_position"];
		m_attributes_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		m_attributes_description[0].offset = offsetof(vertex_attribute_PTNTC, m_position);

		m_attributes_description[1].binding = 0;
		m_attributes_description[1].location = m_attributes_locations["m_texcoord"];
		m_attributes_description[1].format = VK_FORMAT_R16G16_UNORM;
		m_attributes_description[1].offset = offsetof(vertex_attribute_PTNTC, m_texcoord);

		m_attributes_description[2].binding = 0;
		m_attributes_description[2].location = m_attributes_locations["m_normal"];;
		m_attributes_description[2].format = VK_FORMAT_R8G8B8A8_SNORM;
		m_attributes_description[2].offset = offsetof(vertex_attribute_PTNTC, m_normal);

		m_attributes_description[3].binding = 0;
		m_attributes_description[3].location = m_attributes_locations["m_tangent"];
		m_attributes_description[3].format = VK_FORMAT_R8G8B8A8_SNORM;
		m_attributes_description[3].offset = offsetof(vertex_attribute_PTNTC, m_tangent);

		m_attributes_description[4].binding = 0;
		m_attributes_description[4].location = m_attributes_locations["m_color"];
		m_attributes_description[4].format = VK_FORMAT_R8G8B8A8_UNORM;
		m_attributes_description[4].offset = offsetof(vertex_attribute_PTNTC, m_color);

		m_attributes_description[5].binding = 0;
		m_attributes_description[5].location = m_attributes_locations["m_extra"];
		m_attributes_description[5].format = VK_FORMAT_R8G8B8A8_UNORM;
		m_attributes_description[5].offset = offsetof(vertex_attribute_PTNTC, m_unused);

		m_vertex_input_state = vk_initializers::pipeline_vertex_input_state_create_info();
		m_vertex_input_state.vertexBindingDescriptionCount = m_bindings_description.size();
		m_vertex_input_state.pVertexBindingDescriptions = m_bindings_description.data();
		m_vertex_input_state.vertexAttributeDescriptionCount = m_attributes_description.size();
		m_vertex_input_state.pVertexAttributeDescriptions = m_attributes_description.data();

#endif
        
        
#if USED_GRAPHICS_API == METAL_API
        
        m_mtl_vertex_descriptor = mtl_vertex_descriptor::create_PTNTC_vertex_descriptor();
        
#endif

    }
    
    vbo::vertex_declaration_PTNTC::~vertex_declaration_PTNTC()
    {
        
    }
    
    void vbo::vertex_declaration_PTNTC::bind(const std::array<i32, e_shader_attribute_max>& attributes) const
    {
        if(attributes.at(e_shader_attribute_position) >= 0)
        {
            gl::command::enable_vertex_attribute(attributes.at(e_shader_attribute_position));
            gl::command::bind_vertex_attribute(attributes.at(e_shader_attribute_position), 3, gl::constant::f32_t, gl::constant::no,
                                     sizeof(vertex_attribute_PTNTC),
                                     (void*)offsetof(vertex_attribute_PTNTC, m_position));
        }
        if(attributes.at(e_shader_attribute_texcoord) >= 0)
        {
            gl::command::enable_vertex_attribute(attributes.at(e_shader_attribute_texcoord));
            gl::command::bind_vertex_attribute(attributes.at(e_shader_attribute_texcoord), 2, gl::constant::ui16_t, gl::constant::yes,
                                     sizeof(vertex_attribute_PTNTC),
                                     (void*)offsetof(vertex_attribute_PTNTC, m_texcoord));
        }
        if(attributes.at(e_shader_attribute_normal) >= 0)
        {
            gl::command::enable_vertex_attribute(attributes.at(e_shader_attribute_normal));
            gl::command::bind_vertex_attribute(attributes.at(e_shader_attribute_normal), 4, gl::constant::i8_t, gl::constant::yes,
                                     sizeof(vertex_attribute_PTNTC),
                                     (void*)offsetof(vertex_attribute_PTNTC, m_normal));
        }
        if(attributes.at(e_shader_attribute_tangent) >= 0)
        {
            gl::command::enable_vertex_attribute(attributes.at(e_shader_attribute_tangent));
            gl::command::bind_vertex_attribute(attributes.at(e_shader_attribute_tangent), 4, gl::constant::i8_t, gl::constant::yes,
                                     sizeof(vertex_attribute_PTNTC),
                                     (void*)offsetof(vertex_attribute_PTNTC, m_tangent));
        }
        if(attributes.at(e_shader_attribute_color) >= 0)
        {
            gl::command::enable_vertex_attribute(attributes.at(e_shader_attribute_color));
            gl::command::bind_vertex_attribute(attributes.at(e_shader_attribute_color), 4, gl::constant::ui8_t, gl::constant::yes,
                                     sizeof(vertex_attribute_PTNTC),
                                     (void*)offsetof(vertex_attribute_PTNTC, m_color));
        }
    }
    
    void vbo::vertex_declaration_PTNTC::unbind(const std::array<i32, e_shader_attribute_max>& attributes) const
    {
        if(attributes.at(e_shader_attribute_position) >= 0)
        {
            gl::command::disable_vertex_attribute(attributes.at(e_shader_attribute_position));
        }
        if(attributes.at(e_shader_attribute_texcoord) >= 0)
        {
            gl::command::disable_vertex_attribute(attributes.at(e_shader_attribute_texcoord));
        }
        if(attributes.at(e_shader_attribute_normal) >= 0)
        {
            gl::command::disable_vertex_attribute(attributes.at(e_shader_attribute_normal));
        }
        if(attributes.at(e_shader_attribute_tangent) >= 0)
        {
            gl::command::disable_vertex_attribute(attributes.at(e_shader_attribute_tangent));
        }
        if(attributes.at(e_shader_attribute_color) >= 0)
        {
            gl::command::disable_vertex_attribute(attributes.at(e_shader_attribute_color));
        }
    }
    
    std::queue<ui32> vbo::m_handlers_graveyard;
    std::mutex vbo::m_graveyard_mutex;
    
    vbo::vbo(const std::shared_ptr<vertex_declaration>& declaration, ui32 mode, bool is_using_batch) :
    m_declaration(declaration),
    m_handle(0),
    m_allocated_size(declaration->get_size()),
    m_used_size(0),
    m_mode(mode),
    m_is_using_batch(is_using_batch),
    m_min_bound(glm::vec2(0.f)),
    m_max_bound(glm::vec2(0.f)),
    m_version(0)
    {
		assert(m_allocated_size != 0);

        m_type = e_resource_transfering_data_type_vbo;
        
#if USED_GRAPHICS_API == VULKAN_API

		m_staging_buffer = std::make_shared<vk_buffer>();
		VkResult result = vk_utils::create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_staging_buffer, sizeof(vertex_attribute) * m_allocated_size);
		assert(result == VK_SUCCESS);

		m_main_buffer = std::make_shared<vk_buffer>();
		result = vk_utils::create_buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_main_buffer, sizeof(vertex_attribute) * m_allocated_size);
		assert(result == VK_SUCCESS);

#endif
        
#if USED_GRAPHICS_API == METAL_API
        
        if (declaration->get_data())
        {
            m_mtl_buffer_id = std::make_shared<mtl_buffer>(declaration->get_data(), sizeof(vertex_attribute) * m_allocated_size);
        }
        else
        {
            m_mtl_buffer_id = std::make_shared<mtl_buffer>(sizeof(vertex_attribute) * m_allocated_size);
        }
        
#endif

        if(!m_is_using_batch)
        {
            gl::command::create_buffers(1, &m_handle);
        }
    }
    
    vbo::~vbo()
    {
        if(!m_is_using_batch)
        {
            vbo::add_to_graveyard(m_handle);
        }
    }
    
    void vbo::add_to_graveyard(ui32 handler)
    {
        std::lock_guard<std::mutex> guard(m_graveyard_mutex);
        m_handlers_graveyard.push(handler);
    }
    
    ui32 vbo::get_allocated_size() const
    {
        return m_allocated_size;
    }
    
    ui32 vbo::get_used_size() const
    {
        return m_used_size;
    }
    
    glm::vec2 vbo::get_min_bound() const
    {
        return m_min_bound;
    }
    
    glm::vec2 vbo::get_max_bound() const
    {
        return m_max_bound;
    }
    
    ui32 vbo::get_id() const
    {
        return m_handle;
    }
    
    ui32 vbo::get_version() const
    {
        return m_version;
    }
    
    bool vbo::is_using_batch() const
    {
        return m_is_using_batch;
    }
    
    void vbo::unlock(ui32 size, bool submit_to_vram)
    {
        assert(m_declaration->get_data() != nullptr);
        assert(m_allocated_size != 0);
        m_used_size = size > 0 && size < m_allocated_size ? size : m_allocated_size;
        
        if(!m_is_using_batch && submit_to_vram)
        {
            gl::command::bind_buffer(gl::constant::array_buffer, m_handle);
            gl::command::push_buffer_data(gl::constant::array_buffer, sizeof(vertex_attribute) * m_used_size, m_declaration->get_data(), m_mode);
        }

#if USED_GRAPHICS_API == VULKAN_API

		m_staging_buffer->map(sizeof(vertex_attribute) * m_used_size, 0);
		m_staging_buffer->copy_to(m_declaration->get_data(), sizeof(vertex_attribute) * m_used_size);
		m_staging_buffer->unmap();

		vk_utils::copy_buffers(m_staging_buffer, m_main_buffer);

#endif
        
#if USED_GRAPHICS_API == METAL_API
        
        m_mtl_buffer_id->update(m_declaration->get_data(), sizeof(vertex_attribute) * m_used_size);
        
#endif
        
        m_min_bound = glm::vec2(INT16_MAX);
        m_max_bound = glm::vec2(INT16_MIN);
        
        for(i32 i = 0; i < m_used_size; ++i)
        {
            glm::vec2 point = glm::vec2(((vbo::vertex_attribute_P*)m_declaration->get_data())[i].m_position.x,
                                        ((vbo::vertex_attribute_P*)m_declaration->get_data())[i].m_position.y);
            m_min_bound = glm::min(point, m_min_bound);
            m_max_bound = glm::max(point, m_max_bound);
        }
        
        m_version++;
    }
    
    void vbo::bind(const std::array<i32, e_shader_attribute_max>& attributes) const
    {
#if USED_GRAPHICS_API != NO_GRAPHICS_API

        if(m_used_size != 0 && !m_is_using_batch)
        {

#if USED_GRAPHICS_API == VULKAN_API

			VkDeviceSize offsets[] = { 0 };
			ui32 current_image_index = vk_device::get_instance()->get_current_image_index();
			VkCommandBuffer draw_cmd_buffer = vk_device::get_instance()->get_draw_cmd_buffer(current_image_index);
			VkBuffer buffer = m_main_buffer->get_handler();
			vkCmdBindVertexBuffers(draw_cmd_buffer, 0, 1, &buffer, offsets);

#endif
  
            gl::command::bind_buffer(gl::constant::array_buffer, m_handle);
            m_declaration->bind(attributes);
        }

#endif
        
    }
    
    void vbo::unbind(const std::array<i32, e_shader_attribute_max>& attributes) const
    {
        if(m_used_size != 0 && !m_is_using_batch)
        {
            gl::command::bind_buffer(gl::constant::array_buffer, m_handle);
            m_declaration->unbind(attributes);
            gl::command::bind_buffer(gl::constant::array_buffer, NULL);
        }
    }
    
#if USED_GRAPHICS_API == METAL_API
    
    mtl_buffer_shared_ptr vbo::get_mtl_buffer_id() const
    {
        return m_mtl_buffer_id;
    }
    
    mtl_vertex_descriptor_shared_ptr vbo::get_mtl_vertex_descriptor() const
    {
        return m_declaration->get_mtl_vertex_descriptor();
    }
    
#endif

#if USED_GRAPHICS_API == VULKAN_API

	VkPipelineVertexInputStateCreateInfo vbo::get_vertex_input_state() const
	{
		return m_declaration->get_vertex_input_state();
	}

#endif
}
