#include "image_boundary_hud_renderer.hpp"
#include "../image_boundary_hud.hpp"

using namespace xstudio;
using namespace xstudio::ui::viewport;

namespace {

    const char *vertex_shader = R"(
    #version 330 core
    layout (location = 0) in vec4 aPos;
    uniform mat4 to_coord_system;
    uniform mat4 to_canvas;
    uniform float image_aspect;

    void main()
    {
        vec4 rpos = aPos;
        rpos.y = rpos.y/image_aspect;
        gl_Position = (rpos*to_coord_system*to_canvas);
    }
    )";

const char *frag_shader = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 line_colour;
    void main(void)
    {
        FragColor = vec4(line_colour, 1.0f);
    }

    )";
} // namespace

void ImageBoundaryRenderer::render_image_overlay(
    const Imath::M44f &transform_window_to_viewport_space,
    const Imath::M44f &transform_viewport_to_image_space,
    const float /*viewport_du_dpixel*/,
    const float /*device_pixel_ratio*/,
    const xstudio::media_reader::ImageBufPtr &frame) {

    auto data = frame.plugin_blind_data<const ImageBoundaryHUD::HudData>(ImageBoundaryHUD::PLUGIN_UUID);
    if (data && frame) {

        if (!shader_)
            init_overlay_opengl();

        utility::JsonStore shader_params;
        shader_params["to_coord_system"] = transform_viewport_to_image_space.inverse();
        shader_params["to_canvas"]       = transform_window_to_viewport_space;
        shader_params["image_transform_matrix"] = frame.layout_transform();
        shader_params["image_aspect"]           = image_aspect(frame);
        shader_params["line_colour"]            = data->hud_params_["colour"];
        shader_->set_shader_parameters(shader_params);

        glLineWidth(data->hud_params_["width"]);
        shader_->use();
        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(vertex_array_object_);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        shader_->stop_using();
        glBindVertexArray(0);
    }
}

void ImageBoundaryRenderer::init_overlay_opengl() {

    glGenBuffers(1, &vertex_buffer_object_);
    glGenVertexArrays(1, &vertex_array_object_);

    // NOLINT
    static std::array<float, 16> vertices = {
        -1.0,
        -1.0,
        0.0f,
        1.0f,
        1.0,
        -1.0,
        0.0f,
        1.0f,
        1.0,
        1.0,
        0.0f,
        1.0f,
        -1.0,
        1.0,
        0.0f,
        1.0f};

    glBindVertexArray(vertex_array_object_);
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
    // 3. then set our vertex module pointers
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    shader_ = std::make_unique<ui::opengl::GLShaderProgram>(vertex_shader, frag_shader);
}
