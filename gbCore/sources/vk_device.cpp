#include "vk_device.h"

#if USED_GRAPHICS_API == VULKAN_API

#include "vk_surface.h"
#include "vk_initializers.h"
#include "vk_buffer.h"
#include "vk_swap_chain.h"
#include "window_impl.h"

namespace gb
{
	std::shared_ptr<vk_device> vk_device::m_instance = nullptr;

	std::shared_ptr<vk_device> vk_device::get_instance()
	{
		if (!m_instance)
		{
			m_instance = std::make_shared<vk_device>();
		}
		return m_instance;
	}

	vk_device::~vk_device()
	{
		if (m_command_pool)
		{
			vkDestroyCommandPool(m_logical_device, m_command_pool, nullptr);
		}
	}

	void vk_device::construct(VkInstance vk_instance)
	{
		uint32_t devices_count = 0;
		vkEnumeratePhysicalDevices(vk_instance, &devices_count, nullptr);
		assert(devices_count != 0);

		std::vector<VkPhysicalDevice> devices(devices_count);
		vkEnumeratePhysicalDevices(vk_instance, &devices_count, devices.data());

		VkPhysicalDeviceProperties device_properties;
		VkPhysicalDeviceFeatures device_features;

		for (const auto& device : devices)
		{
			vkGetPhysicalDeviceProperties(device, &device_properties);
			vkGetPhysicalDeviceFeatures(device, &device_features);

			m_physical_device = device;
			if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				m_physical_device = device;
				break;
			}
		}
		assert(m_physical_device != VK_NULL_HANDLE);
		vkGetPhysicalDeviceMemoryProperties(m_physical_device, &m_memory_properties);

		vk_queue_family queue_family = get_queue_family();

		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		std::set<int> unique_queue_families = { queue_family.m_graphics_family, queue_family.m_present_family };

		float queue_priority = 1.0f;
		for (int queueFamily : unique_queue_families) {
			VkDeviceQueueCreateInfo queue_create_info = {};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queueFamily;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queue_priority;
			queue_create_infos.push_back(queue_create_info);
		}

		VkDeviceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = static_cast<ui32>(queue_create_infos.size());
		create_info.pQueueCreateInfos = queue_create_infos.data();
		create_info.pEnabledFeatures = &device_features;

		create_info.enabledExtensionCount = static_cast<uint32_t>(m_device_extensions.size());
		create_info.ppEnabledExtensionNames = m_device_extensions.data();

		VkResult result = vkCreateDevice(m_physical_device, &create_info, nullptr, &m_logical_device);
		assert(result == VK_SUCCESS);

		if (result == VK_SUCCESS)
		{
			VkCommandPoolCreateFlags create_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			VkCommandPoolCreateInfo cmd_pool_info = {};
			cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmd_pool_info.queueFamilyIndex = queue_family.m_graphics_family;
			cmd_pool_info.flags = create_flags;
			VkResult result = vkCreateCommandPool(m_logical_device, &cmd_pool_info, nullptr, &m_command_pool);
			assert(result == VK_SUCCESS);
		}

		vkGetDeviceQueue(m_logical_device, queue_family.m_graphics_family, 0, &m_graphics_queue);
		vkGetDeviceQueue(m_logical_device, queue_family.m_present_family, 0, &m_present_queue);

		VkSemaphoreCreateInfo semaphore_create_info = vk_initializers::semaphore_create_info();

		result = vkCreateSemaphore(m_logical_device, &semaphore_create_info, nullptr, &m_present_complete_semaphore);
		assert(result == VK_SUCCESS);

		result = vkCreateSemaphore(m_logical_device, &semaphore_create_info, nullptr, &m_render_complete_semaphore);
		assert(result == VK_SUCCESS);

		result = vkCreateSemaphore(m_logical_device, &semaphore_create_info, nullptr, &m_overlay_complete_semaphore);
		assert(result == VK_SUCCESS);

		m_submit_info = vk_initializers::submit_info();
		m_submit_info.pWaitDstStageMask = &m_submit_pipeline_stages;
		m_submit_info.waitSemaphoreCount = 1;
		m_submit_info.pWaitSemaphores = &m_present_complete_semaphore;
		m_submit_info.signalSemaphoreCount = 1;
		m_submit_info.pSignalSemaphores = &m_render_complete_semaphore;
	}

	void vk_device::create_cmd_buffers()
	{
		m_draw_cmd_buffers.resize(vk_swap_chain::get_instance()->get_images_count());
		VkCommandBufferAllocateInfo cmd_buffer_allocate_info = vk_initializers::command_buffer_allocate_info(m_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<ui32>(m_draw_cmd_buffers.size()));
		VkResult result = vkAllocateCommandBuffers(m_logical_device, &cmd_buffer_allocate_info, m_draw_cmd_buffers.data());
		assert(result == VK_SUCCESS);
	}

	void vk_device::create_frame_buffers(const std::shared_ptr<window_impl>& window)
	{
		const auto resolution = window->get_resolution_size_in_pixels();
		VkFormat depth_format;
		VkBool32 valid_depth_format = vk_device::get_supported_depth_format(&depth_format);
		assert(valid_depth_format);

		VkImageCreateInfo image = {};
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.pNext = NULL;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = depth_format;
		image.extent = { static_cast<ui32>(resolution.x), static_cast<ui32>(resolution.y), 1 };
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		image.flags = 0;

		VkMemoryAllocateInfo mem_alloc = {};
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = 0;
		mem_alloc.memoryTypeIndex = 0;

		VkImageViewCreateInfo depth_stencil_view = {};
		depth_stencil_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		depth_stencil_view.pNext = NULL;
		depth_stencil_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depth_stencil_view.format = depth_format;
		depth_stencil_view.flags = 0;
		depth_stencil_view.subresourceRange = {};
		depth_stencil_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (depth_format == VK_FORMAT_D16_UNORM_S8_UINT || depth_format == VK_FORMAT_D24_UNORM_S8_UINT || depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT)
		{
			depth_stencil_view.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		depth_stencil_view.subresourceRange.baseMipLevel = 0;
		depth_stencil_view.subresourceRange.levelCount = 1;
		depth_stencil_view.subresourceRange.baseArrayLayer = 0;
		depth_stencil_view.subresourceRange.layerCount = 1;

		VkMemoryRequirements mem_requirements;
		VkResult result = vkCreateImage(vk_device::get_instance()->get_logical_device(), &image, nullptr, &m_vk_depth_stencil_image);
		assert(result == VK_SUCCESS);
		vkGetImageMemoryRequirements(vk_device::get_instance()->get_logical_device(), m_vk_depth_stencil_image, &mem_requirements);
		mem_alloc.allocationSize = mem_requirements.size;
		mem_alloc.memoryTypeIndex = vk_device::get_instance()->get_memory_type(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		result = vkAllocateMemory(vk_device::get_instance()->get_logical_device(), &mem_alloc, nullptr, &m_vk_depth_stencil_memory);
		assert(result == VK_SUCCESS);
		result = vkBindImageMemory(vk_device::get_instance()->get_logical_device(), m_vk_depth_stencil_image, m_vk_depth_stencil_memory, 0);
		assert(result == VK_SUCCESS);

		depth_stencil_view.image = m_vk_depth_stencil_image;
		result = vkCreateImageView(vk_device::get_instance()->get_logical_device(), &depth_stencil_view, nullptr, &m_vk_depth_stencil_view);
		assert(result == VK_SUCCESS);

		VkImageView attachments[2];

		attachments[1] = m_vk_depth_stencil_view;

		VkFramebufferCreateInfo frame_buffer_create_info = {};
		frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buffer_create_info.pNext = NULL;
		frame_buffer_create_info.renderPass = vk_swap_chain::get_instance()->get_render_pass();
		frame_buffer_create_info.attachmentCount = 2;
		frame_buffer_create_info.pAttachments = attachments;
		frame_buffer_create_info.width = static_cast<ui32>(resolution.x);
		frame_buffer_create_info.height = static_cast<ui32>(resolution.y);
		frame_buffer_create_info.layers = 1;

		ui32 images_count = vk_swap_chain::get_instance()->get_images_count();
		m_frame_buffers.resize(images_count);

		for (uint32_t i = 0; i < images_count; i++)
		{
			attachments[0] = vk_swap_chain::get_instance()->get_image_view(i);
			result = vkCreateFramebuffer(vk_device::get_instance()->get_logical_device(), &frame_buffer_create_info, nullptr, &m_frame_buffers[i]);
			assert(result == VK_SUCCESS);
		}
	}

	vk_device::vk_queue_family vk_device::get_queue_family()
	{
		vk_queue_family result;
		ui32 queue_familie_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_familie_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(queue_familie_count);
		vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_familie_count, queue_families.data());

		i32 index = 0;
		for (const auto& queue_family : queue_families)
		{
			if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				result.m_graphics_family = index;
			}

			auto surface = vk_surface::get_instance()->get();
			if (surface)
			{
				VkBool32 present_support = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, index, surface, &present_support);

				if (queue_family.queueCount > 0 && present_support)
				{
					result.m_present_family = index;
				}
			}
			else
			{
				assert(false);
			}

			if (result.is_complete())
			{
				break;
			}
			index++;
		}
		return result;
	}

	VkDevice vk_device::get_logical_device()
	{
		return m_logical_device;
	}

	VkPhysicalDevice vk_device::get_physical_device()
	{
		return m_physical_device;
	}

	VkPhysicalDeviceMemoryProperties vk_device::get_memory_properties()
	{
		return m_memory_properties;
	}



	void vk_device::synchronize()
	{
		VkFenceCreateInfo fence_create_info = vk_initializers::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
		m_wait_fences.resize(m_draw_cmd_buffers.size());
		m_frame_numbers.resize(m_draw_cmd_buffers.size(), 0);
		for (auto& fence : m_wait_fences)
		{

			VkResult result = vkCreateFence(m_logical_device, &fence_create_info, nullptr, &fence);
			assert(result == VK_SUCCESS);
		}
	}

	ui32 vk_device::get_memory_type(ui32 type_bits, VkMemoryPropertyFlags properties, VkBool32* mem_type_found)
	{
		for (uint32_t i = 0; i < m_memory_properties.memoryTypeCount; i++)
		{
			if ((type_bits & 1) == 1)
			{
				if ((m_memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					if (mem_type_found)
					{
						*mem_type_found = true;
					}
					return i;
				}
			}
			type_bits >>= 1;
		}

		if (mem_type_found)
		{
			*mem_type_found = false;
			return 0;
		}
		else
		{
			assert(false && "Could not find a matching memory type");
		}
	}

	void vk_device::set_image_layout(VkCommandBuffer cmd_buffer, VkImage image, VkImageLayout old_image_layout, VkImageLayout new_image_layout, VkImageSubresourceRange subresource_range, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
	{
		VkImageMemoryBarrier image_memory_barrier = vk_initializers::image_memory_barrier();
		image_memory_barrier.oldLayout = old_image_layout;
		image_memory_barrier.newLayout = new_image_layout;
		image_memory_barrier.image = image;
		image_memory_barrier.subresourceRange = subresource_range;

		switch (old_image_layout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			image_memory_barrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			image_memory_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			image_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:

			break;
		}

		switch (new_image_layout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			image_memory_barrier.dstAccessMask = image_memory_barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:

			break;
		}

		vkCmdPipelineBarrier(cmd_buffer, src_stage_mask, dst_stage_mask, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
	}

	void vk_device::set_image_layout(VkCommandBuffer cmd_buffer, VkImage image, VkImageAspectFlags aspect_mask, VkImageLayout old_image_layout, VkImageLayout new_image_layout, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
	{
		VkImageSubresourceRange subresource_range = {};
		subresource_range.aspectMask = aspect_mask;
		subresource_range.baseMipLevel = 0;
		subresource_range.levelCount = 1;
		subresource_range.layerCount = 1;
		set_image_layout(cmd_buffer, image, old_image_layout, new_image_layout, subresource_range, src_stage_mask, dst_stage_mask);
	}

	VkCommandPool vk_device::get_command_pool() const
	{
		return m_command_pool;
	}

	VkQueue vk_device::get_graphics_queue() const
	{
		return m_graphics_queue;
	}

	VkQueue vk_device::get_present_queue() const
	{
		return m_present_queue;
	}

	VkBool32 vk_device::get_supported_depth_format(VkFormat *depth_format)
	{
		std::vector<VkFormat> depth_formats = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		for (auto& format : depth_formats)
		{
			VkFormatProperties format_props;
			vkGetPhysicalDeviceFormatProperties(vk_device::get_instance()->get_physical_device(), format, &format_props);
			if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				*depth_format = format;
				return true;
			}
		}
		return false;
	}

	VkSemaphore vk_device::get_present_complete_semaphore()
	{
		return m_present_complete_semaphore;
	}

	VkSemaphore vk_device::get_render_complete_semaphore()
	{
		return m_render_complete_semaphore;
	}

	VkSemaphore vk_device::get_overlay_complete_semaphore()
	{
		return m_overlay_complete_semaphore;
	}

	const std::vector<VkCommandBuffer>& vk_device::get_draw_cmd_buffers() const
	{
		return m_draw_cmd_buffers;
	}

	VkCommandBuffer vk_device::get_draw_cmd_buffer(ui32 index) const
	{
		VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;
		if (index < m_draw_cmd_buffers.size())
		{
			cmd_buffer = m_draw_cmd_buffers.at(index);
		}
		return cmd_buffer;
	}

	std::vector<VkFramebuffer> vk_device::get_frame_buffers() const
	{
		return m_frame_buffers;
	}

	VkFramebuffer vk_device::get_frame_buffer(ui32 index) const
	{
		VkFramebuffer frame_buffer = VK_NULL_HANDLE;
		if (index < m_frame_buffers.size())
		{
			frame_buffer = m_frame_buffers.at(index);
		}
		return frame_buffer;
	}

	const std::vector<VkFence>& vk_device::get_wait_fences() const
	{
		return m_wait_fences;
	}

	VkFence vk_device::get_wait_fence(ui32 index) const
	{
		VkFence fence = VK_NULL_HANDLE;
		if (index < m_wait_fences.size())
		{
			fence = m_wait_fences.at(index);
		}
		return fence;
	}

	ui32 vk_device::get_current_image_index() const
	{
		return m_current_image_index;
	}

	void vk_device::set_current_image_index(ui32 image_index)
	{
		m_current_image_index = image_index;
	}

	ui64 vk_device::get_frame_number(ui32 image_index) const
	{
		return m_frame_numbers.at(image_index);
	}

	void vk_device::begin_frame(ui32 image_index)
	{
		m_frame_numbers.at(image_index)++;
	}
}

#endif
