// SPDX-License-Identifier: Apache-2.0

#include "ffmpeg_pixel_unpack_shader.hpp"

using namespace xstudio;
using namespace xstudio::media_reader;

namespace {
static std::string blankshader{R"(
#version 330 core
uniform int blank_width;
uniform int dummy;

// forward declaration
uvec4 get_image_data_4bytes(int byte_address);

vec4 fetch_rgba_pixel(ivec2 image_coord)
{
    int bytes_per_pixel = 4;
    int pixel_bytes_offset_in_texture_memory = (image_coord.x + image_coord.y*blank_width)*bytes_per_pixel;
    uvec4 c = get_image_data_4bytes(pixel_bytes_offset_in_texture_memory);
    return vec4(float(c.x)/255.0f,float(c.y)/255.0f,float(c.z)/255.0f,1.0f);
}
)"};

static std::string the_shader_yuv = {R"(
#version 410 core
uniform ivec2 texture_dims;
uniform int frame_width_pixels;
uniform int pix_fmt;
uniform int y_linesize;
uniform int u_linesize;
uniform int v_linesize;
uniform int a_linesize;
uniform int y_plane_bytes_offset;
uniform int u_plane_bytes_offset;
uniform int v_plane_bytes_offset;
uniform int a_plane_bytes_offset;
uniform bool half_scale_uvy;
uniform bool half_scale_uvx;
uniform int bits_per_channel;
uniform mat3 yuv_conv;
uniform ivec3 yuv_offsets;
uniform float norm_coeff;

uvec4 get_image_data_4bytes(int byte_address);

int get_image_data_2bytes(int byte_address);

int get_image_data_1byte(int byte_address);

int yuv_tex_lookup_10bit(
	ivec2 image_coord,
	int offset,
	int linestride)
{
    int address = offset + image_coord.x*2 + image_coord.y*linestride;
    return get_image_data_2bytes(address);
}

int yuv_tex_lookup_8bit(
	ivec2 image_coord,
	int offset,
	int linestride)
{
    int address = offset + image_coord.x + image_coord.y*linestride;
    return get_image_data_1byte(address);
}

vec4 fetch_rgba_pixel(ivec2 image_coord)
{
	ivec3 yuv;
	float a = 1.0;

	ivec2 uv_coord = ivec2(
		half_scale_uvx ? image_coord.x >> 1 : image_coord.x,
		half_scale_uvy ? image_coord.y >> 1 : image_coord.y
		);

	if (bits_per_channel == 10 || bits_per_channel == 12) {

		yuv = ivec3(yuv_tex_lookup_10bit(image_coord, y_plane_bytes_offset, y_linesize),
			yuv_tex_lookup_10bit(uv_coord, u_plane_bytes_offset, u_linesize),
			yuv_tex_lookup_10bit(uv_coord, v_plane_bytes_offset, v_linesize)
			);

		if (half_scale_uvx && ((image_coord.x & 1) == 1) && image_coord.x*2 < frame_width_pixels) {

			uv_coord.x = uv_coord.x + 1;
			ivec3 yuv2 = ivec3(yuv.x,
				yuv_tex_lookup_10bit(uv_coord, u_plane_bytes_offset, u_linesize),
				yuv_tex_lookup_10bit(uv_coord, v_plane_bytes_offset, v_linesize)
				);

			yuv = (yuv + yuv2) >> 1;

		}

		if (a_linesize != 0) {
			a = float(yuv_tex_lookup_10bit(image_coord, a_plane_bytes_offset, a_linesize))*norm_coeff;
		}

	} else {

		yuv = ivec3( yuv_tex_lookup_8bit(image_coord, y_plane_bytes_offset, y_linesize),
				yuv_tex_lookup_8bit(uv_coord, u_plane_bytes_offset, u_linesize),
				yuv_tex_lookup_8bit(uv_coord, v_plane_bytes_offset, v_linesize));

	}

	yuv -= yuv_offsets;

	return vec4(vec3(yuv) * yuv_conv * norm_coeff, a); //divide by 1023.0
}
)"};

static std::string the_shader_rgb = {R"(
#version 410 core
uniform ivec2 texture_dims;
uniform int pix_fmt;
uniform int y_linesize;
uniform int u_linesize;
uniform int v_linesize;
uniform int a_linesize;
uniform int y_plane_bytes_offset;
uniform int u_plane_bytes_offset;
uniform int v_plane_bytes_offset;
uniform int a_plane_bytes_offset;
uniform bool half_scale_uvy;
uniform bool half_scale_uvx;
uniform int bits_per_channel;
uniform mat3 yuv_conv;
uniform ivec3 yuv_offsets;
uniform float norm_coeff;

uvec4 get_image_data_4bytes(int byte_address);

int get_image_data_2bytes(int byte_address);

int get_image_data_1byte(int byte_address);

vec4 fetch_rgba_pixel_from_rgb24(ivec2 image_coord, bool bgr)
{
	int address = image_coord.x*3 + image_coord.y*y_linesize;
	uvec3 pix_fmt = bgr ? get_image_data_4bytes(address).zyx : get_image_data_4bytes(address).xyz;
	return vec4(vec3(pix_fmt) * norm_coeff, 1.0);
}

vec4 fetch_rgba_pixel_from_rgba32(ivec2 image_coord)
{
	int address = image_coord.x*4 + image_coord.y*y_linesize;
	uvec4 rgba = get_image_data_4bytes(address);
	if (pix_fmt == 3) { // AV_PIX_FMT_ARGB
		rgba.xyzw = rgba.yzwx;
	} else if (pix_fmt == 4) { // AV_PIX_FMT_RGBA
        //nope
	} else if (pix_fmt == 5) { // AV_PIX_FMT_ABGR
		rgba.xyzw = rgba.wzyx;
	} else if (pix_fmt == 6) { // AV_PIX_FMT_BGRA
		rgba.xyzw = rgba.zyxw;
	}

	return vec4(rgba) * norm_coeff;
}

vec4 fetch_rgba_pixel_from_rgb_48(ivec2 image_coord)
{
    // 2 bytes per channel, 3 channels per pix
	int address = image_coord.x*6 + image_coord.y*y_linesize;
    ivec4 rgba;
    rgba.x = get_image_data_2bytes(address);
    rgba.y = get_image_data_2bytes(address+2);
    rgba.z = get_image_data_2bytes(address+4);
    return vec4(vec3(rgba.xyz) * norm_coeff, 1.0f);
}

vec4 fetch_rgba_pixel_from_rgbf32(ivec2 image_coord)
{
    // 4 bytes per channel, 3 channels per pix
	int address = image_coord.x*12 + image_coord.y*y_linesize;
    ivec4 rgba;
    return vec4(get_image_data_float32(address),
        get_image_data_float32(address+4),
        get_image_data_float32(address+8),
        1.0
    );
}

vec4 fetch_rgba_pixel_from_rgba_64(ivec2 image_coord)
{
    // 2 bytes per channel, 4 channels per pix
	int address = image_coord.x*8 + image_coord.y*y_linesize;
    ivec4 rgba;
    rgba.x = get_image_data_2bytes(address);
    rgba.y = get_image_data_2bytes(address+2);
    rgba.z = get_image_data_2bytes(address+4);
    rgba.w = get_image_data_2bytes(address+6);
    return vec4(rgba) * norm_coeff;
}


vec4 fetch_rgba_pixel_from_gbr_planar(ivec2 image_coord)
{
    // 2 bytes per channel, 3 channels per pix
	int address = image_coord.x*2 + image_coord.y*y_linesize;

    ivec4 rgba;
    rgba.x = get_image_data_2bytes(address+v_plane_bytes_offset);
    rgba.y = get_image_data_2bytes(address+y_plane_bytes_offset);
    rgba.z = get_image_data_2bytes(address+u_plane_bytes_offset);
    if (a_linesize != 0) {
        rgba.w = get_image_data_2bytes(address+a_plane_bytes_offset);
        return vec4(rgba) * norm_coeff;
    } else {
        return vec4(vec3(rgba.xyz) * norm_coeff, 1.0f);
    }

}

vec4 fetch_rgba_pixel(ivec2 image_coord)
{
	if (pix_fmt == 10) {
        return fetch_rgba_pixel_from_rgbf32(image_coord);
    } else if (pix_fmt == 9) {
        return fetch_rgba_pixel_from_rgba_64(image_coord);
     }else if (pix_fmt == 8) {
		return fetch_rgba_pixel_from_rgb_48(image_coord);
	} else if (pix_fmt == 7) {
		return fetch_rgba_pixel_from_gbr_planar(image_coord);
	} else if (pix_fmt > 2) {
		return fetch_rgba_pixel_from_rgba32(image_coord);
	} else {
		return fetch_rgba_pixel_from_rgb24(image_coord, pix_fmt == 2);
	}
}
)"};

}

FFMPegPixelUnpackShader::FFMPegPixelUnpackShader(
    const utility::Uuid &shader_uuid, const ShaderType type)
    : ui::opengl::OpenGLShader(shader_uuid, type==BLANK ? blankshader : type==YUV ? the_shader_yuv : the_shader_rgb) {}