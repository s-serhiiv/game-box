//
//  glCommands.h
//  gbCore
//
//  Created by sergey.sergeev on 8/11/15.
//  Copyright (c) 2015 sergey.sergeev. All rights reserved.
//

#pragma once

// #define USED_GRAPHICS_API NO_GRAPHICS_API

#if USED_GRAPHICS_API != NO_GRAPHICS_API

#if defined(__OSX__)

// #define USED_GRAPHICS_API OPENGL_30_API
#define USED_GRAPHICS_API METAL_API

#elif defined(__WINOS__)

// #define USED_GRAPHICS_API OPENGL_30_API
#define USED_GRAPHICS_API VULKAN_API

#elif defined(__TVOS__)

#define USED_GRAPHICS_API METAL_API

#elif defined(__IOS__)

// #define USED_GRAPHICS_API OPENGL_20_API
#define USED_GRAPHICS_API METAL_API

#endif

#if defined(__IOS__)

#include <QuartzCore/QuartzCore.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#if USED_GRAPHICS_API == OPENGL_20_API

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#elif USED_GRAPHICS_API == OPENGL_30_API

#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>

#endif

#elif defined(__TVOS__)

#include <QuartzCore/QuartzCore.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#if USED_GRAPHICS_API == OPENGL_20_API

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#elif USED_GRAPHICS_API == OPENGL_30_API

#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>

#endif

#elif defined(__OSX__)

#include <OpenGL/OpenGL.h>

#if USED_GRAPHICS_API == OPENGL_20_API

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

#elif USED_GRAPHICS_API == OPENGL_30_API

#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>

#elif USED_GRAPHICS_API == METAL_API

#endif

#elif defined(__WINOS__)

#if USED_GRAPHICS_API == OPENGL_20_API

#define GL_GLEXT_PROTOTYPES 1
#include <gl/glew.h>

#elif USED_GRAPHICS_API == OPENGL_30_API

#define GL_GLEXT_PROTOTYPES 1
#include <gl/glew.h>

#elif USED_GRAPHICS_API == VULKAN_API

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>

#endif

#endif

#endif

#define CASE_STRING_VALUE(GL_ERROR, STRING_ERROR) case GL_ERROR: STRING_ERROR = #GL_ERROR; break;

namespace gb
{
    namespace gl
    {
        namespace constant
        {
            extern ui32 yes;
            extern ui32 no;
            extern ui32 static_draw;
            extern ui32 dynamic_draw;
            extern ui32 f32_t;
            extern ui32 i32_t;
            extern ui32 ui32_t;
            extern ui32 ui24_8_t;
            extern ui32 i16_t;
            extern ui32 ui16_t;
            extern ui32 ui5_6_5_t;
            extern ui32 i8_t;
            extern ui32 ui8_t;
            extern ui32 rgb_t;
            extern ui32 rgba_t;
            extern ui32 texture_2d;
            extern ui32 texture_wrap_s;
            extern ui32 texture_wrap_t;
            extern ui32 texture_mag_filter;
            extern ui32 texture_min_filter;
            extern ui32 linear;
            extern ui32 nearest;
            extern ui32 clamp_to_edge;
            extern ui32 repeat;
            extern ui32 mirrored_repeat;
            extern ui32 linear_mipmap_nearest;
            extern ui32 front;
            extern ui32 back;
            extern ui32 src_color;
            extern ui32 src_alpha;
            extern ui32 one;
            extern ui32 zero;
            extern ui32 one_minus_src_color;
            extern ui32 one_minus_dst_color;
            extern ui32 one_minus_src_alpha;
            extern ui32 one_minus_dst_alpha;
            extern ui32 dst_alpha;
            extern ui32 constant_alpha;
            extern ui32 always;
            extern ui32 equal;
            extern ui32 lequal;
            extern ui32 notequal;
            extern ui32 func_add;
            extern ui32 blend;
            extern ui32 depth_test;
            extern ui32 cull_face;
            extern ui32 stencil_test;
            extern ui32 frame_buffer;
            extern ui32 render_buffer;
            extern ui32 frame_buffer_complete;
            extern ui32 frame_buffer_binding;
            extern ui32 render_buffer_binding;
            extern ui32 render_buffer_width;
            extern ui32 render_buffer_height;
            extern ui32 color_attachment_0;
            extern ui32 depth_attachment;
            extern ui32 stencil_attachment;
            extern ui32 depth_stencil_attachment;
            extern ui32 texture_compare_mode;
            extern ui32 compare_ref_to_texture;
            extern ui32 texture_compare_func;
            extern ui32 depth_component;
            extern ui32 depth_component16;
            extern ui32 depth_component24;
            extern ui32 depth_stencil;
            extern ui32 depth24_stencil8;
            extern ui32 keep;
            extern ui32 replace;
            extern ui32 color_buffer_bit;
            extern ui32 depth_buffer_bit;
            extern ui32 stencil_buffer_bit;
            extern ui32 vertex_shader;
            extern ui32 fragment_shader;
            extern ui32 texture_0;
            extern ui32 red;
            
#if USED_GRAPHICS_API == OPENGL_20_API
            
            extern ui32 luminance;
            
#endif
            
            extern ui32 array_buffer;
            extern ui32 element_array_buffer;
            extern ui32 triangles;
            extern ui32 func_min;
            extern ui32 func_max;
            
            extern ui32 stencil_op_keep;
            extern ui32 stencil_op_zero;
            extern ui32 stencil_op_replace;
            extern ui32 stencil_op_inc_clamp;
            extern ui32 stencil_op_dec_clamp;
            extern ui32 stencil_op_inv;
            extern ui32 stencil_op_inc_wrap;
            extern ui32 stencil_op_dec_wrap;
            extern ui32 less;
            extern ui32 less_equal;
            
            
        };
        
        namespace command
        {
            
#define GL_ERROR_ENABLED (1)
            
            inline ui32 get_error()
            {
                
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
#if defined(GL_ERROR_ENABLED)
                
                ui32 error = glGetError();
                std::string string_error = "";
                switch (error)
                {
                    case GL_NO_ERROR:
                    {
                        
                    }
                        break;
                        
                        CASE_STRING_VALUE(GL_INVALID_ENUM, string_error)
                        CASE_STRING_VALUE(GL_INVALID_VALUE, string_error)
                        CASE_STRING_VALUE(GL_INVALID_OPERATION, string_error)
                        CASE_STRING_VALUE(GL_OUT_OF_MEMORY, string_error)
                        CASE_STRING_VALUE(GL_INVALID_FRAMEBUFFER_OPERATION, string_error)
                        
                    default:
                    {
                        std::stringstream str_stream;
                        str_stream<<"unknown - "<<error;
                        string_error = str_stream.str();
                    }
                }
                if(string_error.length() != 0)
                {
                    std::cout<<"OpenGL error: "<<string_error<<std::endl;
                }
                return error;
                
#else
                
                return 0;
                
#endif
                
#else
                
                return 0;
                
#endif
            };
            
            inline void enable(ui32 caption)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glEnable(caption);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void disable(ui32 caption)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glDisable(caption);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void depth_function(ui32 function)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glDepthFunc(function);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void depth_mask(ui8 flag)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glDepthMask(flag);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void cull_face(ui32 mode)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glCullFace(mode);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void blend_function(ui32 source_factor, ui32 destination_factor)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glBlendFunc(source_factor, destination_factor);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void blend_equation(ui32 equation)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glBlendEquation(equation);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void clear(ui32 mask)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glClear(mask);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void clear_color(f32 red, f32 green, f32 blue, f32 alpha)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glClearColor(red, green, blue, alpha);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void viewport(i32 x, i32 y, i32 width, i32 height)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glViewport(x, y, width, height);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void delete_render_buffers(i32 counts, const ui32 *pointers)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glDeleteRenderbuffers(counts, pointers);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void create_render_buffers(i32 count, ui32 *pointers)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glGenRenderbuffers(count, pointers);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void create_render_buffer_storage(ui32 target, ui32 internalformat, i32 width, i32 height)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glRenderbufferStorage(target, internalformat, width, height);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void bind_frame_buffer(ui32 target, ui32 framebuffer)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glBindFramebuffer(target, framebuffer);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void bind_render_buffer(ui32 target, ui32 renderbuffer)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glBindRenderbuffer(target, renderbuffer);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void delete_frame_buffers(i32 count, const ui32 *pointers)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glDeleteFramebuffers(count, pointers);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void create_frame_buffers(i32 count, ui32 *pointers)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glGenFramebuffers(count, pointers);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void attach_frame_buffer_texture2d(ui32 target, ui32 attachment, ui32 textarget, ui32 texture, i32 level)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glFramebufferTexture2D(target, attachment, textarget, texture, level);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void attach_frame_buffer_render_buffer(ui32 target, ui32 attachment, ui32 renderbuffertarget, ui32 renderbuffer)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void create_textures(i32 count, ui32 *pointers)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glGenTextures(count, pointers);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void delete_textures(i32 count, const ui32 *pointers)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glDeleteTextures(count, pointers);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void bind_texture(ui32 target, ui32 texture)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glBindTexture(target, texture);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void texture_parameter_i(ui32 target, ui32 name, i32 parameter)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glTexParameteri(target, name, parameter);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void texture_image2d(ui32 target, i32 level, i32 internal_format, i32 width, i32 height, i32 border, ui32 format, ui32 type, const void *pixels)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glTexImage2D(target, level, internal_format, width, height, border, format, type, pixels);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void compressed_texture_image2d(ui32 target, i32 level, ui32 internalformat, i32 width, i32 height, i32 border, i32 imageSize, const void *data)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            }
            
            inline void generate_mipmap(ui32 target)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glGenerateMipmap(target);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void create_buffers(i32 count, ui32 *pointers)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glGenBuffers(count, pointers);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void delete_buffers(i32 count, const ui32 *pointers)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glDeleteBuffers(count, pointers);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void bind_buffer(ui32 target, ui32 buffer)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glBindBuffer(target, buffer);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void push_buffer_data(ui32 target, i32 size, const void *data, ui32 usage)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glBufferData(target, size, data, usage);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void push_buffer_sub_data(ui32 target, i32 offset, i32 size, const void *data)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glBufferSubData(target, offset, size, data);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void enable_vertex_attribute(i32 index)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glEnableVertexAttribArray(index);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void disable_vertex_attribute(i32 index)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glDisableVertexAttribArray(index);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void bind_vertex_attribute(ui32 index, i32 size, ui32 type, ui8 normalized, i32 stride, const void *pointer)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glVertexAttribPointer(index, size, type, normalized, stride, pointer);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void set_vertex_attribute_divisor(ui32 index, ui32 divisor)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
#if defined(__OSX__)
                
#if USED_GRAPHICS_API == OPENGL_20_API
                
                glVertexAttribDivisorEXT(index, divisor);
                
#else
                
                glVertexAttribDivisor(index, divisor);
                
#endif
                
#elif defined(__IOS__)
                
#if USED_GRAPHICS_API == OPENGL_20_API
                
                glVertexAttribDivisorEXT(index, divisor);
                
#else
                
                glVertexAttribDivisor(index, divisor);
                
#endif
                
#endif
                
#if defined(__WINOS__)
                
                glVertexAttribDivisor(index, divisor);
                
#endif
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            }
            
            inline void bind_vertex_array(ui32 array)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
#if defined(__OSX__)
                
                glBindVertexArray(array);
                
#endif
                
#if defined(__IOS__) 
#if USED_GRAPHICS_API == OPENGL_30_API
                glBindVertexArray(array);
#elif USED_GRAPHICS_API == OPENGL_20_API
                glBindVertexArrayOES(array);
#endif
#endif
#if defined(__WINOS__)
                glBindVertexArray(array);
#endif 
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void delete_vertex_arrays(i32 count, const ui32 *pointers)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
#if defined(__OSX__)
                
                glDeleteVertexArrays(count, pointers);
                
#endif
                
#if defined(__IOS__) 
                
#if USED_GRAPHICS_API == OPENGL_30_API
                
                glDeleteVertexArrays(count, pointers);
                
#elif USED_GRAPHICS_API == OPENGL_20_API
                
                glDeleteVertexArraysOES(count, pointers);
                
#endif
                
#endif
                
#if defined(__WINOS__)
                
                glDeleteVertexArrays(count, pointers);
                
#endif   
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void create_vertex_arrays(i32 count, ui32 *pointers)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
#if defined(__OSX__)
                
                glGenVertexArrays(count, pointers);
                
#endif
                
#if defined(__IOS__) 
                
#if USED_GRAPHICS_API == OPENGL_30_API
                
                glGenVertexArrays(count, pointers);
                
#elif USED_GRAPHICS_API == OPENGL_20_API
                
                glGenVertexArraysOES(count, pointers);
                
#endif
                
#endif
                
#if defined(__WINOS__)
                
                glGenVertexArrays(count, pointers);
                
#endif   
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void draw_elements(ui32 mode, i32 count, ui32 type, const void *indices)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glDrawElements(mode, count, type, indices);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void draw_elements_instanced(ui32 mode, i32 count, ui32 type, const void *indices, i32 instancecount)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
#if defined(__OSX__)
                
                glDrawElementsInstanced(mode, count, type, indices, instancecount);
                
#elif defined(__IOS__)
                
#if USED_GRAPHICS_API == OPENGL_20_API
                
                glDrawElementsInstancedEXT(mode, count, type, indices, instancecount);
                
#else
                
                glDrawElementsInstanced(mode, count, type, indices, instancecount);
                
#endif
                
#endif
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            }
            
            inline i32 get_uniform_location(ui32 program, const char *name)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                return glGetUniformLocation(program, (char *)name);
#else
                return -1;
                
#endif
            };
            
            inline i32 get_attribute_location(ui32 program, const char *name)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                return glGetAttribLocation(program, (char *)name);
                
#else
                return -1;
                
#endif
            };
            
            inline void get_uniform_matrix_3fv(i32 location, i32 count, ui8 transpose, const f32 *value)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glUniformMatrix3fv(location, count, transpose, value);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void get_uniform_matrix_4fv(i32 location, i32 count, ui8 transpose, const f32 *value)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glUniformMatrix4fv(location, count, transpose, value);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void get_uniform_vector_2fv(i32 location, i32 count, const f32 *value)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glUniform2fv(location, count, value);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void get_uniform_vector_3fv(i32 location, i32 count, const f32 *value)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glUniform3fv(location, count, value);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void get_uniform_vector_4fv(i32 location, i32 count, const f32 *value)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glUniform4fv(location, count, value);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void get_uniform_1f(i32 location, f32 v0)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glUniform1f(location, v0);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void get_uniform_1i(i32 location, i32 v0)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glUniform1i(location, v0);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void set_active_texture(ui32 texture)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glActiveTexture(texture);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void use_program(i32 program)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glUseProgram(program);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void stencil_function(ui32 function, i32 reference, ui32 mask)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glStencilFunc(function, reference, mask);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void stencil_mask(ui32 mask)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glStencilMask(mask);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void color_mask(ui8 red_flag, ui8 green_flag, ui8 blue_flag, ui8 alpha_flag)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glColorMask(red_flag, green_flag, blue_flag, alpha_flag);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline void color_mask(ui8 depth_flag)
            {
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glDepthMask(depth_flag);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
            };
            
            inline ui32 create_shader(ui32 shader_type)
            {
                ui32 handle = 0;
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                handle = glCreateShader(shader_type);
                
#endif
                
#if defined(DEBUG)
                get_error();
#endif
                return handle;
            };
            
            inline void stencil_operation(ui32 fail, ui32 zfail, ui32 zpass)
            {
                
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glStencilOp(fail, zfail, zpass);
                
#if defined(DEBUG)
                
                get_error();
                
#endif
                
                
#endif
            }
            
            inline ui32 check_frame_buffer_status(ui32 value)
            {
                
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                return glCheckFramebufferStatus(value);
                
#endif
                
                return gl::constant::frame_buffer_complete;
                
            };
            
            inline void read_pixels(i32 x, i32 y, ui32 width, ui32 height, ui32 format, ui32 type, void* pixels)
            {
                
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glReadPixels(x, y, width, height, format, type, pixels);
                
#if defined(DEBUG)
                
                get_error();
                
#endif
                
#endif
                
            };
            
            inline void delete_program(ui32 handle)
            {
                
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
                
                glDeleteProgram(handle);
                
#if defined(DEBUG)
                
                get_error();
                
#endif
                
#endif
                
            }
        };
    };
};
