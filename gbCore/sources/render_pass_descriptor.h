//
//  render_pass_descriptor.h
//  gbCore
//

#pragma once

#include "main_headers.h"
#include "declarations.h"
#include "ws_technique_configuration.h"
#include "ss_technique_configuration.h"

#if USED_GRAPHICS_API != NO_GRAPHICS_API

namespace gb
{
    class i_render_pass_descriptor_impl
    {
    public:

        virtual ~i_render_pass_descriptor_impl() = default;

        virtual void bind() = 0;
        virtual void unbind() = 0;

        virtual std::vector<texture_shared_ptr> get_color_attachments_texture() const = 0;
        virtual texture_shared_ptr get_depth_attachment_texture() const = 0;
    };

    class render_pass_descriptor
    {
    private:

        std::shared_ptr<i_render_pass_descriptor_impl> m_render_pass_descriptor_impl = nullptr;

    public:

        render_pass_descriptor(const std::shared_ptr<i_render_pass_descriptor_impl>& render_pass_descriptor_impl);

        ~render_pass_descriptor();

        static render_pass_descriptor_shared_ptr construct_ws_render_pass_descriptor(const std::shared_ptr<ws_technique_configuration>& configuration);
        static render_pass_descriptor_shared_ptr construct_ss_render_pass_descriptor(const std::shared_ptr<ss_technique_configuration>& configuration);
        static render_pass_descriptor_shared_ptr construct_output_render_pass_descriptor(const std::string& name);

        void bind();
        void unbind();

        std::vector<texture_shared_ptr> get_color_attachments_texture() const;
        texture_shared_ptr get_depth_attachment_texture() const;
    };
};

#endif
