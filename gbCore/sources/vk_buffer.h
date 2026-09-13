#pragma once

#include "main_headers.h"

#if USED_GRAPHICS_API == VULKAN_API

#include "resource.h"

namespace gb
{
	class vk_buffer
	{
	private:

		VkDevice m_vk_device = VK_NULL_HANDLE;
		VkBuffer m_vk_buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_vk_memory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo m_vk_descriptor;
		VkDeviceSize m_vk_size = 0;
		VkDeviceSize m_vk_alignment = 0;
		void* m_mapped_data = nullptr;

		VkBufferUsageFlags m_vk_usage_flags;
		VkMemoryPropertyFlags m_vk_memory_property_flags;

	public:

		vk_buffer();
		~vk_buffer();

		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void unmap();

		VkResult bind(VkDeviceSize offset = 0);

		void set_descriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		void copy_to(void* data, VkDeviceSize size);

		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		VkBuffer get_handler() const;
		VkDeviceMemory get_memory() const;
		const VkDescriptorBufferInfo& get_descriptor() const;
		void* get_mapped_data() const;

		VkDeviceSize get_size() const;
		VkDeviceSize get_aligment() const;

		void set_handler(VkBuffer handler);
		void set_memory(VkDeviceMemory memory);

		void set_aligment(VkDeviceSize aligment);
		void set_size(VkDeviceSize size);
		void set_usage_flags(VkBufferUsageFlags usage_flags);
		void set_memory_property_flags(VkMemoryPropertyFlags memory_property_flags);
	};
};

#endif

