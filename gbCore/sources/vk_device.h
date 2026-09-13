#pragma once

#include "main_headers.h"

#if USED_GRAPHICS_API == VULKAN_API

#include "declarations.h"

namespace gb
{
	class vk_device
	{
	public:

		struct vk_queue_family
		{
			i32 m_graphics_family = -1;
			i32 m_present_family = -1;

			bool is_complete() const
			{
				return m_graphics_family >= 0 && m_present_family >= 0;
			};
		};

	private:

		static std::shared_ptr<vk_device> m_instance;

		VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
		VkDevice m_logical_device = VK_NULL_HANDLE;

		VkQueue m_graphics_queue = VK_NULL_HANDLE;
		VkQueue m_present_queue = VK_NULL_HANDLE;

		VkCommandPool m_command_pool = VK_NULL_HANDLE;

		VkPhysicalDeviceMemoryProperties m_memory_properties;

		std::vector<const char*> m_device_extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		VkSemaphore m_present_complete_semaphore;
		VkSemaphore m_render_complete_semaphore;
		VkSemaphore m_overlay_complete_semaphore;

		VkPipelineStageFlags m_submit_pipeline_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo m_submit_info;

		VkImage m_vk_depth_stencil_image = VK_NULL_HANDLE;
		VkDeviceMemory m_vk_depth_stencil_memory = VK_NULL_HANDLE;
		VkImageView m_vk_depth_stencil_view = VK_NULL_HANDLE;

		std::vector<VkFramebuffer> m_frame_buffers;

		std::vector<VkCommandBuffer> m_draw_cmd_buffers;
		std::vector<VkFence> m_wait_fences;

		ui32 m_current_image_index = 0;
		std::vector<ui64> m_frame_numbers;

	protected:

	public:

		~vk_device();

		static std::shared_ptr<vk_device> get_instance();

		void construct(VkInstance vk_instance);
		void create_cmd_buffers();
		void create_frame_buffers(const std::shared_ptr<window_impl>& window);
		void synchronize();

		VkDevice get_logical_device();
		VkPhysicalDevice get_physical_device();

		VkPhysicalDeviceMemoryProperties get_memory_properties();
		ui32 get_memory_type(ui32 type_bits, VkMemoryPropertyFlags properties, VkBool32 *mem_type_found = nullptr);

		vk_queue_family get_queue_family();

		VkCommandPool get_command_pool() const;

		VkQueue get_graphics_queue() const;
		VkQueue get_present_queue() const;

		void set_image_layout(VkCommandBuffer cmd_buffer, VkImage image, VkImageLayout old_image_layout, VkImageLayout new_image_layout, VkImageSubresourceRange subresource_range, VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		void set_image_layout(VkCommandBuffer cmd_buffer, VkImage image, VkImageAspectFlags aspect_mask, VkImageLayout old_image_layout, VkImageLayout new_image_layout, VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	
		static VkBool32 get_supported_depth_format(VkFormat *depth_format);

		const std::vector<VkCommandBuffer>& get_draw_cmd_buffers() const;
		VkCommandBuffer get_draw_cmd_buffer(ui32 index) const;

		std::vector<VkFramebuffer> get_frame_buffers() const;
		VkFramebuffer get_frame_buffer(ui32 index) const;
		
		const std::vector<VkFence>& get_wait_fences() const;
		VkFence get_wait_fence(ui32 index) const;

		VkSemaphore get_present_complete_semaphore();
		VkSemaphore get_render_complete_semaphore();
		VkSemaphore get_overlay_complete_semaphore();

		ui32 get_current_image_index() const;
		void set_current_image_index(ui32 image_index);
		ui64 get_frame_number(ui32 image_index) const;
		void begin_frame(ui32 image_index);
	};
};

#endif
