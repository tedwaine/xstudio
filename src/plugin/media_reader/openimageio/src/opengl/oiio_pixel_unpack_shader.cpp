// SPDX-License-Identifier: Apache-2.0

#include "oiio_pixel_unpack_shader.hpp"

using namespace xstudio;
using namespace xstudio::media_reader;

namespace {
// Supports: 8-bit, 16-bit UINT, 16-bit HALF, float32
static std::string oiio_shader_code{R"(
#version 410 core
uniform int width;
uniform int height;
uniform int bytes_per_channel;  // 1=8bit, 2=16bit, 4=float
uniform int is_half_float;      // 0=UINT16, 1=HALF float
uniform int has_alpha;          // 0=no alpha, 1=has alpha
uniform int channel_r_start;
uniform int channel_g_start;
uniform int channel_b_start;
uniform int channel_a_start;

// Forward declarations
int get_image_data_1byte(int byte_address);
int get_image_data_2bytes(int byte_address);
vec2 get_image_data_2floats(int byte_address);
float get_image_data_float32(int byte_address);

vec4 fetch_rgba_pixel(ivec2 image_coord)
{
    float R = 0.0f, G = 0.0f, B = 0.0f, A = 1.0f;

    int r_coord = (image_coord.x + image_coord.y * width) * bytes_per_channel + channel_r_start;
    int g_coord = (image_coord.x + image_coord.y * width) * bytes_per_channel + channel_g_start;
    int b_coord = (image_coord.x + image_coord.y * width) * bytes_per_channel + channel_b_start;
    int a_coord = (image_coord.x + image_coord.y * width) * bytes_per_channel + channel_a_start;

    if (bytes_per_channel == 1) {
        R = get_image_data_1byte(r_coord) / 255.0;
        G = get_image_data_1byte(g_coord) / 255.0;
        B = get_image_data_1byte(b_coord) / 255.0;
        if (has_alpha == 1) A = get_image_data_1byte(a_coord) / 255.0;
    }
    else if (bytes_per_channel == 2) {
        if (is_half_float == 1) {
            R = get_image_data_2floats(r_coord).x;
            G = get_image_data_2floats(g_coord).x;
            B = get_image_data_2floats(b_coord).x;
            if (has_alpha == 1) A = get_image_data_2floats(a_coord).x;
        }
        else {
            R = get_image_data_2bytes(r_coord) / 65535.0;
            G = get_image_data_2bytes(g_coord) / 65535.0;
            B = get_image_data_2bytes(b_coord) / 65535.0;
            if (has_alpha == 1) A = get_image_data_2bytes(a_coord) / 65535.0;
        }
    }
    else if (bytes_per_channel == 4) {
        R = get_image_data_float32(r_coord);
        G = get_image_data_float32(g_coord);
        B = get_image_data_float32(b_coord);
        if (has_alpha == 1) A = get_image_data_float32(a_coord);
    }

    return vec4(R, G, B, A);
}
)"};
}

OIIOPixelUnpackShader::OIIOPixelUnpackShader(const utility::Uuid &shader_uuid)
    : ui::opengl::OpenGLShader(shader_uuid, oiio_shader_code) {}
