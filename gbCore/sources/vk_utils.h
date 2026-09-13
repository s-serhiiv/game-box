#pragma once

#include "main_headers.h"
#include "declarations.h"

#if USED_GRAPHICS_API == VULKAN_API

namespace gb
{
	class vk_buffer;
	class vk_utils
	{
	public:

		static VkResult create_buffer(VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_property_flags, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data = nullptr);
		static VkResult create_buffer(VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_property_flags, const std::shared_ptr<vk_buffer>& buffer, VkDeviceSize size, void *data = nullptr);
	
		static VkCommandBuffer create_command_buffer(VkCommandBufferLevel level, bool begin = false);
		static void flush_command_buffer(VkCommandBuffer command_buffer, VkQueue queue, bool free = true);

		static void copy_buffers(const std::shared_ptr<vk_buffer>& buffer_src, const std::shared_ptr<vk_buffer>& buffer_dst);
	};

#define VK_CHECK(f) \
{ \
	VkResult result = (f); \
	if (result != VK_SUCCESS) \
	{ \
		std::cout << "Fatal : VkResult is \"" << result << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl; \
		assert(result == VK_SUCCESS); \
	} \
}

#define GET_VK_INSTANCE_PROC_ADDR(instance, entrypoint) \
{ \
	fp##entrypoint = reinterpret_cast<PFN_vk##entrypoint>(vkGetInstanceProcAddr(instance, "vk"#entrypoint)); \
	if (fp##entrypoint == NULL) \
	{ \
		std::cout << "Fatal : Vulkan instance entrypoint is missing: vk"#entrypoint << std::endl; \
		exit(1); \
	} \
}

#define GET_VK_DEVICE_PROC_ADDR(device, entrypoint) \
{ \
	fp##entrypoint = reinterpret_cast<PFN_vk##entrypoint>(vkGetDeviceProcAddr(device, "vk"#entrypoint)); \
	if (fp##entrypoint == NULL) \
	{ \
		std::cout << "Fatal : Vulkan device entrypoint is missing: vk"#entrypoint << std::endl; \
		exit(1); \
	} \
}
};

#endif
