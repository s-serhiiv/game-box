#include "vk_utils.h"

#if USED_GRAPHICS_API == VULKAN_API

#include "vk_initializers.h"
#include "vk_buffer.h"
#include "vk_device.h"

#define VK_FLAGS_NONE 0
#define DEFAULT_FENCE_TIMEOUT 100000000000

namespace gb
{
	VkResult vk_utils::create_buffer(VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_property_flags, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data)
	{
		VkBufferCreateInfo buffer_create_info = vk_initializers::buffer_create_info(usage_flags, size);
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkResult result = vkCreateBuffer(vk_device::get_instance()->get_logical_device(), &buffer_create_info, nullptr, buffer);
		assert(result == VK_SUCCESS);

		VkMemoryRequirements mem_requirements;
		VkMemoryAllocateInfo mem_alloc = vk_initializers::memory_allocate_info();
		vkGetBufferMemoryRequirements(vk_device::get_instance()->get_logical_device(), *buffer, &mem_requirements);
		mem_alloc.allocationSize = mem_requirements.size;
		mem_alloc.memoryTypeIndex = vk_device::get_instance()->get_memory_type(mem_requirements.memoryTypeBits, memory_property_flags);
		result = vkAllocateMemory(vk_device::get_instance()->get_logical_device(), &mem_alloc, nullptr, memory);
		assert(result == VK_SUCCESS);

		if (data != nullptr)
		{
			void *mapped;
			result = vkMapMemory(vk_device::get_instance()->get_logical_device(), *memory, 0, size, 0, &mapped);
			assert(result == VK_SUCCESS);
			memcpy(mapped, data, size);
			if ((memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			{
				VkMappedMemoryRange mapped_range = vk_initializers::mapped_memory_range();
				mapped_range.memory = *memory;
				mapped_range.offset = 0;
				mapped_range.size = size;
				vkFlushMappedMemoryRanges(vk_device::get_instance()->get_logical_device(), 1, &mapped_range);
			}
			vkUnmapMemory(vk_device::get_instance()->get_logical_device(), *memory);
		}

		result = vkBindBufferMemory(vk_device::get_instance()->get_logical_device(), *buffer, *memory, 0);
		assert(result == VK_SUCCESS);

		return result;
	}

	VkResult vk_utils::create_buffer(VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_property_flags, const std::shared_ptr<vk_buffer>& buffer, VkDeviceSize size, void *data)
	{
		VkBufferCreateInfo buffer_create_info = vk_initializers::buffer_create_info(usage_flags, size);
		auto handler = buffer->get_handler();
		VkResult result = vkCreateBuffer(vk_device::get_instance()->get_logical_device(), &buffer_create_info, nullptr, &handler);
		assert(result == VK_SUCCESS);
		buffer->set_handler(handler);

		VkMemoryRequirements mem_requirements;
		VkMemoryAllocateInfo mem_alloc = vk_initializers::memory_allocate_info();
		vkGetBufferMemoryRequirements(vk_device::get_instance()->get_logical_device(), handler, &mem_requirements);
		mem_alloc.allocationSize = mem_requirements.size;
		mem_alloc.memoryTypeIndex = vk_device::get_instance()->get_memory_type(mem_requirements.memoryTypeBits, memory_property_flags);
		auto memory = buffer->get_memory();
		result = vkAllocateMemory(vk_device::get_instance()->get_logical_device(), &mem_alloc, nullptr, &memory);
		assert(result == VK_SUCCESS);
		buffer->set_memory(memory);

		buffer->set_aligment(mem_requirements.alignment);
		buffer->set_size(size);
		buffer->set_usage_flags(usage_flags);
		buffer->set_memory_property_flags(memory_property_flags);

		if (data != nullptr)
		{
			result = buffer->map();
			assert(result == VK_SUCCESS);
			auto mapped_data = buffer->get_mapped_data();
			memcpy(mapped_data, data, size);
			buffer->unmap();
		}
		buffer->set_descriptor();

		return buffer->bind();
	}

	VkCommandBuffer vk_utils::create_command_buffer(VkCommandBufferLevel level, bool begin)
	{
		VkCommandBufferAllocateInfo cmd_buffer_allocate_info = vk_initializers::command_buffer_allocate_info(vk_device::get_instance()->get_command_pool(), level, 1);

		VkCommandBuffer cmd_buffer;
		VkResult result = vkAllocateCommandBuffers(vk_device::get_instance()->get_logical_device(), &cmd_buffer_allocate_info, &cmd_buffer);
		assert(result == VK_SUCCESS);

		if (begin)
		{
			VkCommandBufferBeginInfo cmd_buffer_info = vk_initializers::command_buffer_begin_info();
			result = vkBeginCommandBuffer(cmd_buffer, &cmd_buffer_info);
			assert(result == VK_SUCCESS);
		}

		return cmd_buffer;
	}

	void vk_utils::flush_command_buffer(VkCommandBuffer command_buffer, VkQueue queue, bool free)
	{
		if (command_buffer == VK_NULL_HANDLE)
		{
			return;
		}

		VkResult result = vkEndCommandBuffer(command_buffer);
		assert(result == VK_SUCCESS);

		VkSubmitInfo submit_info = vk_initializers::submit_info();
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		VkFenceCreateInfo fence_info = vk_initializers::fence_create_info(VK_FLAGS_NONE);
		VkFence fence;
		result = vkCreateFence(vk_device::get_instance()->get_logical_device(), &fence_info, nullptr, &fence);
		assert(result == VK_SUCCESS);

		result = vkQueueSubmit(queue, 1, &submit_info, fence);
		assert(result == VK_SUCCESS);

		result = vkWaitForFences(vk_device::get_instance()->get_logical_device(), 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
		assert(result == VK_SUCCESS);

		vkDestroyFence(vk_device::get_instance()->get_logical_device(), fence, nullptr);

		if (free)
		{
			vkFreeCommandBuffers(vk_device::get_instance()->get_logical_device(), vk_device::get_instance()->get_command_pool(), 1, &command_buffer);
		}
	}

	void vk_utils::copy_buffers(const std::shared_ptr<vk_buffer>& buffer_src, const std::shared_ptr<vk_buffer>& buffer_dst)
	{
		VkCommandBuffer copy_cmd = vk_utils::create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		VkBufferCopy copy_region{};
		copy_region.size = buffer_src->get_size();
		VkBuffer src_handler = buffer_src->get_handler();
		VkBuffer dst_handler = buffer_dst->get_handler();
		vkCmdCopyBuffer(copy_cmd, src_handler, dst_handler, 1, &copy_region);
		buffer_dst->set_handler(dst_handler);
		VkQueue copy_queue = vk_device::get_instance()->get_graphics_queue();
		vk_utils::flush_command_buffer(copy_cmd, copy_queue);
	}
}

#endif
