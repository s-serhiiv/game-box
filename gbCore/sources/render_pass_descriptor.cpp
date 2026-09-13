//
//  render_pass_descriptor.cpp
//  gbCore
//

#include "render_pass_descriptor.h"

#if USED_GRAPHICS_API != NO_GRAPHICS_API

#if USED_GRAPHICS_API == VULKAN_API

#include "vk_render_pass_descriptor.h"

#elif USED_GRAPHICS_API == METAL_API

#include "mtl_device.h"
#include "mtl_render_pass_descriptor.h"

#endif

namespace gb
{
#if USED_GRAPHICS_API == METAL_API

    class render_pass_descriptor_mtl_impl : public i_render_pass_descriptor_impl
    {
    private:

        mtl_render_pass_descriptor_shared_ptr m_render_pass_descriptor = nullptr;

    public:

        render_pass_descriptor_mtl_impl(const mtl_render_pass_descriptor_shared_ptr& render_pass_descriptor) :
        m_render_pass_descriptor(render_pass_descriptor)
        {

        }

        void bind() override
        {
            m_render_pass_descriptor->bind();
        }

        void unbind() override
        {
            m_render_pass_descriptor->unbind();
        }

        std::vector<texture_shared_ptr> get_color_attachments_texture() const override
        {
            return m_render_pass_descriptor->get_color_attachments_texture();
        }

        texture_shared_ptr get_depth_attachment_texture() const override
        {
            return nullptr;
        }
    };

#endif

    render_pass_descriptor::render_pass_descriptor(const std::shared_ptr<i_render_pass_descriptor_impl>& render_pass_descriptor_impl) :
    m_render_pass_descriptor_impl(render_pass_descriptor_impl)
    {

    }

    render_pass_descriptor::~render_pass_descriptor()
    {

    }

    render_pass_descriptor_shared_ptr render_pass_descriptor::construct_ws_render_pass_descriptor(const std::shared_ptr<ws_technique_configuration>& configuration)
    {
#if USED_GRAPHICS_API == VULKAN_API

        const auto render_pass_descriptor_impl = vk_render_pass_descriptor::construct_ws_render_pass_descriptor(configuration);

#elif USED_GRAPHICS_API == METAL_API

        const auto render_pass_descriptor_impl = std::make_shared<render_pass_descriptor_mtl_impl>(mtl_render_pass_descriptor::construct_ws_render_pass_descriptor(configuration));

#else

        std::shared_ptr<i_render_pass_descriptor_impl> render_pass_descriptor_impl = nullptr;

#endif

        return render_pass_descriptor_impl ? std::make_shared<render_pass_descriptor>(render_pass_descriptor_impl) : nullptr;
    }

    render_pass_descriptor_shared_ptr render_pass_descriptor::construct_ss_render_pass_descriptor(const std::shared_ptr<ss_technique_configuration>& configuration)
    {
#if USED_GRAPHICS_API == VULKAN_API

        const auto render_pass_descriptor_impl = vk_render_pass_descriptor::construct_ss_render_pass_descriptor(configuration);

#elif USED_GRAPHICS_API == METAL_API

        const auto render_pass_descriptor_impl = std::make_shared<render_pass_descriptor_mtl_impl>(mtl_render_pass_descriptor::construct_ss_render_pass_descriptor(configuration));

#else

        std::shared_ptr<i_render_pass_descriptor_impl> render_pass_descriptor_impl = nullptr;

#endif

        return render_pass_descriptor_impl ? std::make_shared<render_pass_descriptor>(render_pass_descriptor_impl) : nullptr;
    }

    render_pass_descriptor_shared_ptr render_pass_descriptor::construct_output_render_pass_descriptor(const std::string& name)
    {
#if USED_GRAPHICS_API == VULKAN_API

        const auto render_pass_descriptor_impl = vk_render_pass_descriptor::construct_output_render_pass_descriptor(name);

#elif USED_GRAPHICS_API == METAL_API

        const auto render_pass_descriptor_impl = std::make_shared<render_pass_descriptor_mtl_impl>(mtl_render_pass_descriptor::construct_output_render_pass_descriptor(name, mtl_device::get_instance()->get_mtl_raw_color_attachment_ptr(), mtl_device::get_instance()->get_mtl_raw_depth_stencil_attachment_ptr()));

#else

        std::shared_ptr<i_render_pass_descriptor_impl> render_pass_descriptor_impl = nullptr;

#endif

        return render_pass_descriptor_impl ? std::make_shared<render_pass_descriptor>(render_pass_descriptor_impl) : nullptr;
    }

    void render_pass_descriptor::bind()
    {
        assert(m_render_pass_descriptor_impl);
        m_render_pass_descriptor_impl->bind();
    }

    void render_pass_descriptor::unbind()
    {
        assert(m_render_pass_descriptor_impl);
        m_render_pass_descriptor_impl->unbind();
    }

    std::vector<texture_shared_ptr> render_pass_descriptor::get_color_attachments_texture() const
    {
        assert(m_render_pass_descriptor_impl);
        return m_render_pass_descriptor_impl->get_color_attachments_texture();
    }

    texture_shared_ptr render_pass_descriptor::get_depth_attachment_texture() const
    {
        assert(m_render_pass_descriptor_impl);
        return m_render_pass_descriptor_impl->get_depth_attachment_texture();
    }
}

#endif
