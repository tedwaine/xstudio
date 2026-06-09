// SPDX-License-Identifier: Apache-2.0

#include "ppm_pixel_unpack_shader.hpp"

using namespace xstudio;
using namespace xstudio::media_reader;

namespace {
static std::string myshader{R"(
#version 330 core
uniform int width;
uniform int bytes_per_channel;

// forward declaration
uvec4 get_image_data_4bytes(int byte_address);

vec4 fetch_pixel_8bit(ivec2 image_coord)
{
    int address = (image_coord.x + image_coord.y*width)*3;
    uvec4 c = get_image_data_4bytes(address);
    return vec4(float(c.x)/255.0f,float(c.y)/255.0f,float(c.z)/255.0f,1.0f);
}

vec4 fetch_pixel_16bit(ivec2 image_coord)
{
    int address = (image_coord.x + image_coord.y*width)*6;
    uvec4 c = get_image_data_4bytes(address);
    return vec4(float(c.x)/65535.0f,float(c.y)/65535.0f,float(c.z)/65535.0f,1.0f);
}

vec4 fetch_rgba_pixel(ivec2 image_coord)
{
    if (bytes_per_channel == 1) {
        return fetch_pixel_8bit(image_coord);
    } else {
        return fetch_pixel_16bit(image_coord);
    }
}
)"};
}

PPMPixelUnpackShader::PPMPixelUnpackShader(const utility::Uuid &shader_uuid)
    : ui::opengl::OpenGLShader(shader_uuid, myshader) {}
