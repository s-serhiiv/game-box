#include "vk_swap_chain.h"

#if USED_GRAPHICS_API == VULKAN_API

#include "window_impl.h"
#include "vk_device.h"
#include "vk_surface.h"
#include "vk_utils.h"

namespace gb
{
	std::shared_ptr<vk_swap_chain> vk_swap_chain::m_instance = nullptr;

	vk_swap_chain::~vk_swap_chain()
	{

	}

	std::shared_ptr<vk_swap_chain> vk_swap_chain::get_instance()
	{
		if (!m_instance)
		{
			m_instance = std::make_shared<vk_swap_chain>();
		}
		return m_instance;
	}

	void vk_swap_chain::construct(VkInstance vk_instance, const std::shared_ptr<window_impl>& window)
	{
		vk_swap_chain_support_details swap_chain_support = query_swap_chain_support();

		VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swap_chain_support.m_formats);
		VkPresentModeKHR present_mode = choose_swap_present_mode(swap_chain_support.m_present_modes);
		VkExtent2D extent = choose_swap_extent(swap_chain_support.m_capabilities, window);

		uint32_t images_count = swap_chain_support.m_capabilities.minImageCount + 1;
		if (swap_chain_support.m_capabilities.maxImageCount > 0 && images_count > swap_chain_support.m_capabilities.maxImageCount)
		{
			images_count = swap_chain_support.m_capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = vk_surface::get_instance()->get();

		create_info.minImageCount = images_count;
		create_info.imageFormat = surface_format.format;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		vk_device::vk_queue_family queue_family = vk_device::get_instance()->get_queue_family();
		ui32 queue_families[] = { static_cast<ui32>(queue_family.m_graphics_family), static_cast<ui32>(queue_family.m_present_family) };

		if (queue_family.m_graphics_family != queue_family.m_present_family)
		{
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_families;
		}
		else
		{
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0;
			create_info.pQueueFamilyIndices = nullptr;
		}

		create_info.preTransform = swap_chain_support.m_capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;

		create_info.oldSwapchain = VK_NULL_HANDLE;

		VkResult result = vkCreateSwapchainKHR(vk_device::get_instance()->get_logical_device(), &create_info, nullptr, &m_swap_chain);
		assert(result == VK_SUCCESS);

		vkGetSwapchainImagesKHR(vk_device::get_instance()->get_logical_device(), m_swap_chain, &images_count, nullptr);
		m_images.resize(images_count);
		vkGetSwapchainImagesKHR(vk_device::get_instance()->get_logical_device(), m_swap_chain, &images_count, m_images.data());

		m_image_format = surface_format.format;
		m_extent = extent;

		m_buffers.resize(images_count);
		for (uint32_t i = 0; i < images_count; i++)
		{
			VkImageViewCreateInfo color_attachment_view = {};
			color_attachment_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			color_attachment_view.pNext = NULL;
			color_attachment_view.format = m_image_format;
			color_attachment_view.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			color_attachment_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			color_attachment_view.subresourceRange.baseMipLevel = 0;
			color_attachment_view.subresourceRange.levelCount = 1;
			color_attachment_view.subresourceRange.baseArrayLayer = 0;
			color_attachment_view.subresourceRange.layerCount = 1;
			color_attachment_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
			color_attachment_view.flags = 0;

			m_buffers[i].m_image = m_images[i];

			color_attachment_view.image = m_buffers[i].m_image;

			result = vkCreateImageView(vk_device::get_instance()->get_logical_device(), &color_attachment_view, nullptr, &m_buffers[i].m_view);
			assert(result == VK_SUCCESS);
		}

		VkFormat depth_format;
		VkBool32 valid_depth_format = vk_device::get_supported_depth_format(&depth_format);
		assert(valid_depth_format);

		std::array<VkAttachmentDescription, 2> attachments = {};
		attachments[0].format = m_image_format;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		attachments[1].format = depth_format;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference color_attachment_ref = {};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_ref = {};
		depth_attachment_ref.attachment = 1;
		depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass_description = {};
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount = 1;
		subpass_description.pColorAttachments = &color_attachment_ref;
		subpass_description.pDepthStencilAttachment = &depth_attachment_ref;
		subpass_description.inputAttachmentCount = 0;
		subpass_description.pInputAttachments = nullptr;
		subpass_description.preserveAttachmentCount = 0;
		subpass_description.pPreserveAttachments = nullptr;
		subpass_description.pResolveAttachments = nullptr;

		std::array<VkSubpassDependency, 2> dependencies = {};

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		render_pass_info.pAttachments = attachments.data();
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass_description;
		render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
		render_pass_info.pDependencies = dependencies.data();

		result = vkCreateRenderPass(vk_device::get_instance()->get_logical_device(), &render_pass_info, nullptr, &m_render_pass);
		assert(result == VK_SUCCESS);

		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount = 0;
		pipeline_layout_create_info.pushConstantRangeCount = 0;

		result = vkCreatePipelineLayout(vk_device::get_instance()->get_logical_device(), &pipeline_layout_create_info, nullptr, &m_pipeline_layout);
		assert(result == VK_SUCCESS);
	}

	void vk_swap_chain::connect(VkInstance vk_instance)
	{
		GET_VK_INSTANCE_PROC_ADDR(vk_instance, GetPhysicalDeviceSurfaceSupportKHR);
		GET_VK_INSTANCE_PROC_ADDR(vk_instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
		GET_VK_INSTANCE_PROC_ADDR(vk_instance, GetPhysicalDeviceSurfaceFormatsKHR);
		GET_VK_INSTANCE_PROC_ADDR(vk_instance, GetPhysicalDeviceSurfacePresentModesKHR);
		GET_VK_DEVICE_PROC_ADDR(vk_device::get_instance()->get_logical_device(), CreateSwapchainKHR);
		GET_VK_DEVICE_PROC_ADDR(vk_device::get_instance()->get_logical_device(), DestroySwapchainKHR);
		GET_VK_DEVICE_PROC_ADDR(vk_device::get_instance()->get_logical_device(), GetSwapchainImagesKHR);
		GET_VK_DEVICE_PROC_ADDR(vk_device::get_instance()->get_logical_device(), AcquireNextImageKHR);
		GET_VK_DEVICE_PROC_ADDR(vk_device::get_instance()->get_logical_device(), QueuePresentKHR);
	}

	VkSwapchainKHR vk_swap_chain::get() const
	{
		return m_swap_chain;
	}

	vk_swap_chain::vk_swap_chain_support_details vk_swap_chain::query_swap_chain_support()
	{
		vk_swap_chain_support_details details;
		auto surface = vk_surface::get_instance()->get();

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_device::get_instance()->get_physical_device(), surface, &details.m_capabilities);

		ui32 formats_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(vk_device::get_instance()->get_physical_device(), surface, &formats_count, nullptr);

		if (formats_count != 0) 
		{
			details.m_formats.resize(formats_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(vk_device::get_instance()->get_physical_device(), surface, &formats_count, details.m_formats.data());
		}

		ui32 present_modes_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(vk_device::get_instance()->get_physical_device(), surface, &present_modes_count, nullptr);

		if (present_modes_count != 0)
		{
			details.m_present_modes.resize(present_modes_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(vk_device::get_instance()->get_physical_device(), surface, &present_modes_count, details.m_present_modes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR vk_swap_chain::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& formats)
	{
		if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
		{
			return { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}
		for (const auto& format : formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}
		return formats[0];
	}

	VkPresentModeKHR vk_swap_chain::choose_swap_present_mode(const std::vector<VkPresentModeKHR> present_modes)
	{
		VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& present_mode : present_modes) 
		{
			if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return present_mode;
			}
			else if (present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				mode = present_mode;
			}
		}
		return mode;
	}

	VkExtent2D vk_swap_chain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, const std::shared_ptr<window_impl>& window)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			const auto resolution = window->get_resolution_size_in_pixels();
			VkExtent2D actual_extent = { static_cast<ui32>(resolution.x), static_cast<ui32>(resolution.y) };

			actual_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
			actual_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actual_extent.height));

			return actual_extent;
		}
	}

	const std::vector<VkImage>& vk_swap_chain::get_images() const
	{
		return m_images;
	}

	const ui32 vk_swap_chain::get_images_count() const
	{
		return static_cast<ui32>(m_images.size());
	}

	VkImageView vk_swap_chain::get_image_view(ui32 index) const
	{
		assert(index < m_buffers.size());
		return m_buffers.at(index).m_view;
	}

	VkFormat vk_swap_chain::get_swap_chain_image_format() const
	{
		return m_image_format;
	}

	VkExtent2D vk_swap_chain::get_swap_chain_extent() const
	{
		return m_extent;
	}

	VkRenderPass vk_swap_chain::get_render_pass() const
	{
		return m_render_pass;
	}

	VkPipelineLayout vk_swap_chain::get_pipeline_layout() const
	{
		return m_pipeline_layout;
	}

	VkResult vk_swap_chain::acquire_next_image(VkSemaphore present_complete_semaphore, ui32 *image_index)
	{
		return fpAcquireNextImageKHR(vk_device::get_instance()->get_logical_device(), m_swap_chain, UINT64_MAX, present_complete_semaphore, (VkFence) nullptr, image_index);
	}

	VkResult vk_swap_chain::queue_present(VkQueue queue, ui32 image_index, VkSemaphore wait_semaphore)
	{
		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext = NULL;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &m_swap_chain;
		present_info.pImageIndices = &image_index;
		if (wait_semaphore != VK_NULL_HANDLE)
		{
			present_info.pWaitSemaphores = &wait_semaphore;
			present_info.waitSemaphoreCount = 1;
		}
		return fpQueuePresentKHR(queue, &present_info);
	}
}

#endif
