// SPDX-License-Identifier: Apache-2.0

#include "pdf_pixel_unpack_shader.hpp"

using namespace xstudio;
using namespace xstudio::media_reader;

namespace {
static std::string myshader{R"(
#version 330 core
uniform int width;

// forward declaration
uvec4 get_image_data_4bytes(int byte_address);

vec4 fetch_rgba_pixel(ivec2 image_coord)
{
    int address = (image_coord.x + image_coord.y*width)*3;
    uvec4 c = get_image_data_4bytes(address);
    return vec4(float(c.x)/255.0f,float(c.y)/255.0f,float(c.z)/255.0f,1.0f);
}
)"};

static std::string myshader_transparent{R"(
#version 330 core
uniform int width;

// forward declaration
uvec4 get_image_data_4bytes(int byte_address);

vec4 fetch_rgba_pixel(ivec2 image_coord)
{
    int address = (image_coord.x + image_coord.y * width) * 4;
    uvec4 c = get_image_data_4bytes(address);
    return vec4(float(c.x)/255.0f,float(c.y)/255.0f,float(c.z)/255.0f,float(c.w)/255.0f);
}
)"};
}

PDFPixelUnpackShader::PDFPixelUnpackShader(const utility::Uuid &shader_uuid, const bool transparent)
    : ui::opengl::OpenGLShader(shader_uuid, transparent ? myshader_transparent : myshader) {}
