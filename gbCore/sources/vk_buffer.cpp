#include "vk_buffer.h"

#if USED_GRAPHICS_API == VULKAN_API

#include "vk_device.h"

namespace gb
{
	vk_buffer::vk_buffer()
	{
		m_vk_device = vk_device::get_instance()->get_logical_device();
		assert(m_vk_device != VK_NULL_HANDLE);
	}

	vk_buffer::~vk_buffer()
	{
		unmap();
		if (m_vk_buffer)
		{
			vkDestroyBuffer(m_vk_device, m_vk_buffer, nullptr);
		}
		if (m_vk_memory)
		{
			vkFreeMemory(m_vk_device, m_vk_memory, nullptr);
		}
	}

	VkResult vk_buffer::map(VkDeviceSize size, VkDeviceSize offset)
	{
		return vkMapMemory(m_vk_device, m_vk_memory, offset, size, 0, &m_mapped_data);
	}

	void vk_buffer::unmap()
	{
		if (m_mapped_data)
		{
			vkUnmapMemory(m_vk_device, m_vk_memory);
			m_mapped_data = nullptr;
		}
	}

	VkResult vk_buffer::bind(VkDeviceSize offset)
	{
		return vkBindBufferMemory(m_vk_device, m_vk_buffer, m_vk_memory, offset);
	}

	void vk_buffer::set_descriptor(VkDeviceSize size, VkDeviceSize offset)
	{
		m_vk_descriptor.offset = offset;
		m_vk_descriptor.buffer = m_vk_buffer;
		m_vk_descriptor.range = size;
	}

	void vk_buffer::copy_to(void* data, VkDeviceSize size)
	{
		assert(m_mapped_data);
		memcpy(m_mapped_data, data, size);
	}

	VkResult vk_buffer::flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mapped_range = {};
		mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mapped_range.memory = m_vk_memory;
		mapped_range.offset = offset;
		mapped_range.size = size;
		return vkFlushMappedMemoryRanges(m_vk_device, 1, &mapped_range);
	}

	VkResult vk_buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mapped_range = {};
		mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mapped_range.memory = m_vk_memory;
		mapped_range.offset = offset;
		mapped_range.size = size;
		return vkInvalidateMappedMemoryRanges(m_vk_device, 1, &mapped_range);
	}

	VkBuffer vk_buffer::get_handler() const
	{
		return m_vk_buffer;
	}

	VkDeviceMemory vk_buffer::get_memory() const
	{
		return m_vk_memory;
	}

	const VkDescriptorBufferInfo& vk_buffer::get_descriptor() const
	{
		return m_vk_descriptor;
	}

	void* vk_buffer::get_mapped_data() const
	{
		return m_mapped_data;
	}

	VkDeviceSize vk_buffer::get_size() const
	{
		return m_vk_size;
	}

	VkDeviceSize vk_buffer::get_aligment() const
	{
		return m_vk_alignment;
	}

	void vk_buffer::set_handler(VkBuffer handler)
	{
		m_vk_buffer = handler;
	}

	void vk_buffer::set_memory(VkDeviceMemory memory)
	{
		m_vk_memory = memory;
	}

	void vk_buffer::set_aligment(VkDeviceSize aligment)
	{
		m_vk_alignment = aligment;
	}

	void vk_buffer::set_size(VkDeviceSize size)
	{
		m_vk_size = size;
	}

	void vk_buffer::set_usage_flags(VkBufferUsageFlags usage_flags)
	{
		m_vk_usage_flags = usage_flags;
	}

	void vk_buffer::set_memory_property_flags(VkMemoryPropertyFlags memory_property_flags)
	{
		m_vk_memory_property_flags = memory_property_flags;
	}
}

#endif
