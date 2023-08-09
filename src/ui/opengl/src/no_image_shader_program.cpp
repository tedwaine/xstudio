// SPDX-License-Identifier: Apache-2.0
#include <sstream>

#include "xstudio/ui/opengl/no_image_shader_program.hpp"

using namespace xstudio::ui::opengl;

namespace {

static const std::string basic_vertex_shd = R"(
#version 330 core
layout (location = 0) in vec4 aPos;
uniform mat4 to_coord_system;
uniform mat4 to_canvas;

void main(void)
{
    vec4 rpos = aPos*to_coord_system;
    gl_Position = aPos*to_canvas;
}
)";

static const std::string colour_transforms = R"(
#version 330 core
vec4 colour_transforms(vec4 rgba_in)
{
    return rgba_in;
}
)";

static const std::string basic_frag_shd = R"(
#version 150
vec4 fetch_rgba_pixel(ivec2 image_coord)
{
    return vec4(0.1f,0.4f,0.8f,1.0f);
})";

} // namespace

NoImageShaderProgram::NoImageShaderProgram()
    : GLShaderProgram(basic_vertex_shd, colour_transforms, basic_frag_shd) {}
