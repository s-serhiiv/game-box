//
//  gl_commands.cpp
//  gbCore
//
//  Created by serhii.s on 3/4/19.
//  Copyright © 2019 sergey.sergeev. All rights reserved.
//

#include "main_headers.h"

namespace gb
{
    namespace gl
    {
        namespace constant
        {
            
#if USED_GRAPHICS_API == OPENGL_20_API || USED_GRAPHICS_API == OPENGL_30_API
            
            ui32 yes = GL_TRUE;
            ui32 no = GL_FALSE;
            ui32 static_draw = GL_STATIC_DRAW;
            ui32 dynamic_draw = GL_DYNAMIC_DRAW;
            ui32 f32_t = GL_FLOAT;
            ui32 i32_t = GL_INT;
            ui32 ui32_t = GL_UNSIGNED_INT;
            
#if USED_GRAPHICS_API == OPENGL_30_API
            
            ui32 ui24_8_t = GL_UNSIGNED_INT_24_8;
            
#elif USED_GRAPHICS_API == OPENGL_20_API
            
            
            ui32 ui24_8_t = GL_UNSIGNED_INT_24_8_OES;
#endif
            
            ui32 i16_t = GL_SHORT;
            ui32 ui16_t = GL_UNSIGNED_SHORT;
            ui32 ui5_6_5_t = GL_UNSIGNED_SHORT_5_6_5;
            ui32 i8_t = GL_BYTE;
            ui32 ui8_t = GL_UNSIGNED_BYTE;
            ui32 rgb_t = GL_RGB;
            ui32 rgba_t = GL_RGBA;
            ui32 texture_2d = GL_TEXTURE_2D;
            ui32 texture_wrap_s = GL_TEXTURE_WRAP_S;
            ui32 texture_wrap_t = GL_TEXTURE_WRAP_T;
            ui32 texture_mag_filter = GL_TEXTURE_MAG_FILTER;
            ui32 texture_min_filter = GL_TEXTURE_MIN_FILTER;
            ui32 linear = GL_LINEAR;
            ui32 nearest = GL_NEAREST;
            ui32 clamp_to_edge = GL_CLAMP_TO_EDGE;
            ui32 repeat = GL_REPEAT;
            ui32 mirrored_repeat = GL_MIRRORED_REPEAT;
            ui32 linear_mipmap_nearest = GL_LINEAR_MIPMAP_NEAREST;
            ui32 front = GL_FRONT;
            ui32 back = GL_BACK;
            ui32 src_color = GL_SRC_COLOR;
            ui32 src_alpha = GL_SRC_ALPHA;
            ui32 one = GL_ONE;
            ui32 zero = GL_ZERO;
            ui32 one_minus_src_color = GL_ONE_MINUS_SRC_COLOR;
            ui32 one_minus_dst_color = GL_ONE_MINUS_DST_COLOR;
            ui32 one_minus_src_alpha = GL_ONE_MINUS_SRC_ALPHA;
            ui32 one_minus_dst_alpha = GL_ONE_MINUS_DST_ALPHA;
            ui32 dst_alpha = GL_DST_ALPHA;
            ui32 constant_alpha = GL_CONSTANT_ALPHA;
            ui32 always = GL_ALWAYS;
            ui32 equal = GL_EQUAL;
            ui32 lequal = GL_LEQUAL;
            ui32 notequal = GL_NOTEQUAL;
            ui32 func_add = GL_FUNC_ADD;
            ui32 blend = GL_BLEND;
            ui32 depth_test = GL_DEPTH_TEST;
            ui32 cull_face = GL_CULL_FACE;
            ui32 stencil_test = GL_STENCIL_TEST;
            ui32 frame_buffer = GL_FRAMEBUFFER;
            ui32 render_buffer = GL_RENDERBUFFER;
            ui32 frame_buffer_complete = GL_FRAMEBUFFER_COMPLETE;
            ui32 frame_buffer_binding = GL_FRAMEBUFFER_BINDING;
            ui32 render_buffer_binding = GL_RENDERBUFFER_BINDING;
            ui32 render_buffer_width = GL_RENDERBUFFER_WIDTH;
            ui32 render_buffer_height = GL_RENDERBUFFER_HEIGHT;
            ui32 color_attachment_0 = GL_COLOR_ATTACHMENT0;
            ui32 depth_attachment = GL_DEPTH_ATTACHMENT;
            ui32 stencil_attachment = GL_STENCIL_ATTACHMENT;
            
#if USED_GRAPHICS_API == OPENGL_30_API
            
            ui32 depth_stencil_attachment = GL_DEPTH_STENCIL_ATTACHMENT;
            ui32 texture_compare_mode = GL_TEXTURE_COMPARE_MODE;
            ui32 compare_ref_to_texture = GL_COMPARE_REF_TO_TEXTURE;
            ui32 texture_compare_func = GL_TEXTURE_COMPARE_FUNC;
            
#elif USED_GRAPHICS_API == OPENGL_20_API
            
            ui32 depth_stencil_attachment = 0;
            ui32 texture_compare_mode = GL_TEXTURE_COMPARE_MODE_EXT;
            ui32 compare_ref_to_texture = GL_COMPARE_REF_TO_TEXTURE_EXT;
            ui32 texture_compare_func = GL_TEXTURE_COMPARE_FUNC_EXT;
            
#endif
            
            ui32 depth_component = GL_DEPTH_COMPONENT;
            ui32 depth_component16 = GL_DEPTH_COMPONENT16;
            
#if USED_GRAPHICS_API == OPENGL_30_API
            
            ui32 depth_component24 = GL_DEPTH_COMPONENT24;
            ui32 depth_stencil = GL_DEPTH_STENCIL;
            ui32 depth24_stencil8 = GL_DEPTH24_STENCIL8;
            
#elif USED_GRAPHICS_API == OPENGL_20_API
            
            ui32 depth_component24 = GL_DEPTH_COMPONENT24_OES;
            ui32 depth_stencil = GL_DEPTH_STENCIL_OES;
            ui32 depth24_stencil8 = GL_DEPTH24_STENCIL8_OES;
            
#endif
            
            ui32 keep = GL_KEEP;
            ui32 replace = GL_REPLACE;
            ui32 color_buffer_bit = GL_COLOR_BUFFER_BIT;
            ui32 depth_buffer_bit = GL_DEPTH_BUFFER_BIT;
            ui32 stencil_buffer_bit = GL_STENCIL_BUFFER_BIT;
            ui32 vertex_shader = GL_VERTEX_SHADER;
            ui32 fragment_shader = GL_FRAGMENT_SHADER;
            ui32 texture_0 = GL_TEXTURE0;
            
#if USED_GRAPHICS_API == OPENGL_30_API
            
            ui32 red = GL_RED;
            ui32 luminance = 0;
            
#endif
            
#if USED_GRAPHICS_API == OPENGL_20_API
            
            ui32 red = 0;
            ui32 luminance = GL_LUMINANCE;

#endif
            
            ui32 array_buffer = GL_ARRAY_BUFFER;
            ui32 element_array_buffer = GL_ELEMENT_ARRAY_BUFFER;
            ui32 triangles = GL_TRIANGLES;
            ui32 func_min = GL_MIN;
            ui32 func_max = GL_MAX;
            ui32 stencil_op_keep = GL_KEEP;
            ui32 stencil_op_zero = GL_ZERO;
            ui32 stencil_op_replace = GL_REPLACE;
            ui32 stencil_op_inc_clamp = GL_INCR;
            ui32 stencil_op_dec_clamp = GL_DECR;
            ui32 stencil_op_inv = GL_INVERT;
            ui32 stencil_op_inc_wrap = GL_INCR_WRAP;
            ui32 stencil_op_dec_wrap = GL_DECR_WRAP;
            ui32 less = GL_LESS;
            ui32 less_equal = GL_LEQUAL;
            
#else
            
            ui32 yes = 1;
            ui32 no = 2;
            ui32 static_draw = 3;
            ui32 dynamic_draw = 4;
            ui32 f32_t = 5;
            ui32 i32_t = 6;
            ui32 ui32_t = 7;
            ui32 ui24_8_t = 8;
            ui32 i16_t = 9;
            ui32 ui16_t = 10;
            ui32 ui5_6_5_t = 11;
            ui32 i8_t = 12;
            ui32 ui8_t = 13;
            ui32 rgb_t = 14;
            ui32 rgba_t = 15;
            ui32 texture_2d = 16;
            ui32 texture_wrap_s = 17;
            ui32 texture_wrap_t = 18;
            ui32 texture_mag_filter = 19;
            ui32 texture_min_filter = 20;
            ui32 linear = 21;
            ui32 nearest = 22;
            ui32 clamp_to_edge = 23;
            ui32 repeat = 24;
            ui32 mirrored_repeat = 25;
            ui32 linear_mipmap_nearest = 26;
            ui32 front = 27;
            ui32 back = 28;
            ui32 src_color = 29;
            ui32 src_alpha = 30;
            ui32 one = 31;
            ui32 zero = 32;
            ui32 one_minus_src_color = 33;
            ui32 one_minus_dst_color = 34;
            ui32 one_minus_src_alpha = 35;
            ui32 one_minus_dst_alpha = 36;
            ui32 dst_alpha = 37;
            ui32 constant_alpha = 38;
            ui32 always = 39;
            ui32 equal = 40;
            ui32 lequal = 41;
            ui32 notequal = 42;
            ui32 func_add = 43;
            ui32 blend = 44;
            ui32 depth_test = 45;
            ui32 cull_face = 46;
            ui32 stencil_test = 47;
            ui32 frame_buffer = 48;
            ui32 render_buffer = 49;
            ui32 frame_buffer_complete = 50;
            ui32 frame_buffer_binding = 51;
            ui32 render_buffer_binding = 52;
            ui32 render_buffer_width = 53;
            ui32 render_buffer_height = 54;
            ui32 color_attachment_0 = 55;
            ui32 depth_attachment = 56;
            ui32 stencil_attachment = 57;
            ui32 depth_stencil_attachment = 58;
            ui32 texture_compare_mode = 59;
            ui32 compare_ref_to_texture = 60;
            ui32 texture_compare_func = 61;
            ui32 depth_component = 62;
            ui32 depth_component16 = 63;
            ui32 depth_component24 = 64;
            ui32 depth_stencil = 65;
            ui32 depth24_stencil8 = 66;
            ui32 keep = 67;
            ui32 replace = 68;
            ui32 color_buffer_bit = 69;
            ui32 depth_buffer_bit = 70;
            ui32 stencil_buffer_bit = 71;
            ui32 vertex_shader = 72;
            ui32 fragment_shader = 73;
            ui32 texture_0 = 74;
            ui32 red = 75;
            ui32 luminance = 76;
            ui32 array_buffer = 77;
            ui32 element_array_buffer = 78;
            ui32 triangles = 79;
            ui32 func_min = 80;
            ui32 func_max = 81;
            ui32 stencil_op_keep = 82;
            ui32 stencil_op_zero = 83;
            ui32 stencil_op_replace = 84;
            ui32 stencil_op_inc_clamp = 85;
            ui32 stencil_op_dec_clamp = 86;
            ui32 stencil_op_inv = 87;
            ui32 stencil_op_inc_wrap = 88;
            ui32 stencil_op_dec_wrap = 89;
            ui32 less = 90;
            ui32 less_equal = 91;
            
#endif
            
        };
    };
};
