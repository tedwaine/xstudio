// SPDX-License-Identifier: Apache-2.0

#include "exr_pixel_unpack_shader.hpp"

using namespace xstudio;
using namespace xstudio::media_reader;

namespace {
static std::string shader{R"(
#version 410 core
uniform int width;
uniform int height;
uniform int num_channels;
uniform int pix_type_r;
uniform int pix_type_g;
uniform int pix_type_b;
uniform int pix_type_a;
uniform int bytes_per_pixel;

//Overall fragment shader provides these ones
//uniform ivec2 image_bounds_min;
//uniform ivec2 image_bounds_max;

// we need to forward declare this function, which is defined by the base
// gl shader class
vec2 get_image_data_2floats(int byte_address);
float get_image_data_float32(int byte_address);

vec4 fetch_rgba_pixel(ivec2 image_coord)
{
    if (image_coord.x < image_bounds_min.x || image_coord.x >= image_bounds_max.x) return vec4(0.0,0.0,0.0,0.0);
	if (image_coord.y < image_bounds_min.y || image_coord.y >= image_bounds_max.y) return vec4(0.0,0.0,0.0,0.0);

    int pixel_address_bytes = ((image_coord.x-image_bounds_min.x) + (image_coord.y-image_bounds_min.y)*(image_bounds_max.x-image_bounds_min.x))*bytes_per_pixel;

    float R = 0.9;
    float G = 0.4;
    float B = 0.0;
    float A = 1.0;

    vec2 pixRG = get_image_data_2floats(pixel_address_bytes);

    if(pix_type_r == 1) {
        R = pixRG.x;
        pixel_address_bytes = pixel_address_bytes+2;
    } else if(pix_type_r == 2) {
        R = get_image_data_float32(pixel_address_bytes);
        pixel_address_bytes = pixel_address_bytes+4;
    }

    if(num_channels == 1) {
        // 1 channels, assume luminance
        return vec4(R, R, R, 1.0);
    }

    if(pix_type_g == 1) {
        G = pixRG.y;
        pixel_address_bytes = pixel_address_bytes+2;
    } else if(pix_type_g == 2) {
        G = get_image_data_float32(pixel_address_bytes);
        pixel_address_bytes = pixel_address_bytes+4;
    }

    if(num_channels == 2) {
        // 2 channels, assume luminance/alpha
        return vec4(R, R, R, G);
    }

    vec2 pixBA = get_image_data_2floats(pixel_address_bytes);

    if(pix_type_b == 1) {
        B = pixBA.x;
        pixel_address_bytes = pixel_address_bytes+2;
    } else if(pix_type_b == 2) {
        B = get_image_data_float32(pixel_address_bytes);
        pixel_address_bytes = pixel_address_bytes+4;
    }

    if(num_channels == 3) {
        return vec4(R, G, B, 1.0);
    }

    if(pix_type_a == 1) {
        A = pixBA.y;
    } else if(pix_type_a == 2) {
        A = get_image_data_float32(pixel_address_bytes);
    }

    return vec4(R, G, B, A);
}
)"};
}

EXRPixelUnpackShader::EXRPixelUnpackShader(const utility::Uuid &shader_uuid)
    : ui::opengl::OpenGLShader(shader_uuid, shader) {}
