//
//  shader_compiler_glsl.h
//  gbCore
//
//  Created by Sergey Sergeev on 8/12/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#ifndef shader_compiler_glsl_h
#define shader_compiler_glsl_h

#include "main_headers.h"

namespace gb
{
#if USED_GRAPHICS_API == VULKAN_API

	struct shader_custom_uniform_desc
	{
		std::string m_name;
		std::string m_type;
		ui32 m_offset = 0;
		ui32 m_element_size = 0;
		ui32 m_stride = 0;
		ui32 m_array_size = 1;
	};

#endif

    class shader_compiler_glsl
    {
    private:
        
    protected:
        
        static std::string m_vs_shader_header;
        static std::string m_fs_shader_header;
        
    public:
        
        shader_compiler_glsl(void) = default;
        ~shader_compiler_glsl(void) = default;
 
#if USED_GRAPHICS_API == VULKAN_API

		static VkPipelineShaderStageCreateInfo compile(const std::string& source_code, ui32 shader_type, std::string* out_message = nullptr, bool* out_success = nullptr);
		static std::string convert_to_vulkan_source(const std::string& source_code, ui32 binding, std::vector<shader_custom_uniform_desc>* uniforms = nullptr);

#else

		static ui32 compile(const std::string& source_code, ui32 shader_type, std::string* out_message = nullptr, bool* out_success = nullptr);

#endif
       
        static ui32 link(ui32 vs_handle, ui32 fs_handle, std::string* out_message = nullptr, bool* out_success = nullptr);
    };
};

#endif
