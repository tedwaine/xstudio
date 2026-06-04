#ifdef __apple__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#include "media_metadata_renderer.hpp"

namespace {
const char *vertex_shader = R"(
    #version 330 core
    layout (location = 0) in vec4 aPos;
    uniform mat4 tform;
    uniform vec2 bdb_min;
    uniform vec2 bdb_max;

    void main()
    {
        vec4 rpos = aPos;
        rpos.x = bdb_min.x + (bdb_max.x - bdb_min.x)*(rpos.x + 1.0f)*0.5f;
        rpos.y = bdb_min.y + (bdb_max.y - bdb_min.y)*(rpos.y + 1.0f)*0.5f;
        gl_Position = (rpos*tform);
    }
    )";

const char *frag_shader = R"(
    #version 330 core
    out vec4 FragColor;
    uniform float opacity;

    void main(void)
    {
        FragColor = vec4(0.0, 0.0, 0.0, opacity);
    }

    )";

inline size_t __hash_combine(size_t lhs, size_t rhs) {
    lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    return lhs;
}

} // namespace

using namespace xstudio;
using namespace xstudio::ui::viewport;

void MediaMetadataRenderer::render_image_overlay(
    const Imath::M44f &transform_window_to_viewport_space,
    const Imath::M44f &transform_viewport_to_image_space,
    const float viewport_du_dpixel,
    const float device_pixel_ratio,
    const xstudio::media_reader::ImageBufPtr &frame) {

    // auto t0 = utility::clock::now();

    if (!text_renderer_) {
        init_overlay_opengl();
    }

    auto render_data =
        frame.plugin_blind_data(utility::Uuid("f3e7c2db-2578-45d6-8ad5-743779057a63"));
    const auto *data = dynamic_cast<const MediaMetadata *>(render_data.get());
    if (!data) {
        return;
    }


    // the gl viewport corresponds to the parent window size.
    // TODO: For Qt6 viewport dims will be passed into this function, we can't
    // read viewport like this
    std::array<int, 4> gl_viewport;
    glGetIntegerv(GL_VIEWPORT, gl_viewport.data());
    const auto viewport_width  = (float)gl_viewport[2];
    const auto viewport_height = (float)gl_viewport[3];

    if (display_settings_ != data->display_settings_) {

        display_settings_ = data->display_settings_;
    }
    glDisable(GL_BLEND);


    // draw BG boxes
    if (display_settings_->bg_opacity > 0.0f) {
        glBindVertexArray(vao_);
        utility::JsonStore bdb_param;
        bdb_param["opacity"] = display_settings_->bg_opacity;
        shader_->use();
        glEnableVertexAttribArray(0);
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);
        auto p = data->positions_.begin();
        for (const auto &bdb : data->bdbs_) {
            // 'anchor' position of text: -1.0,-1.0 is top left of viewport,
            // 1.0, 1.0 is bottom right
            Imath::V4f anchor = *p;
            p++;
            // the gl viewport is set to the entire window size, not the viewport
            // within the window. So we use 'transform_window_to_viewport_space'
            // matrx here ...
            Imath::M44f m1, m2;
            if (data->grid_layout_) {
                m1 = transform_viewport_to_image_space.inverse();
                m1.translate(Imath::V3f(anchor.x, -anchor.y / image_aspect(frame), 0.0f));
                m1 *= transform_window_to_viewport_space;
            } else {
                anchor *= transform_window_to_viewport_space;
                m2.scale(
                    Imath::V3f(viewport_width / 1920.0f, -viewport_height / 1920.0f, 1.0f));
                m2.translate(Imath::V3f(-anchor.x / anchor.w, -anchor.y / anchor.w, 0.0f));
            }
            bdb_param["bdb_min"] = bdb.min;
            bdb_param["bdb_max"] = bdb.max;
            bdb_param["tform"]   = m1 * (m2.inverse());
            shader_->set_shader_parameters(bdb_param);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glDisable(GL_BLEND);
        shader_->stop_using();
        glDisableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    // draw text - note, if images are in grid layout, the metadata text is anchored
    // to the image. Otherwise the text is anchored to the viewport boundary
    auto p = data->positions_.begin();
    for (const auto &v : data->verts_) {

        Imath::V4f anchor = *p;
        p++;

        Imath::M44f m1, m2;
        if (data->grid_layout_) {
            m1 = transform_viewport_to_image_space.inverse();
            m1.translate(Imath::V3f(anchor.x, -anchor.y / image_aspect(frame), 0.0f));
            m1 *= transform_window_to_viewport_space;
        } else {
            anchor *= transform_window_to_viewport_space;
            m2.scale(Imath::V3f(viewport_width / 1920.0f, -viewport_height / 1920.0f, 1.0f));
            m2.translate(Imath::V3f(-anchor.x / anchor.w, -anchor.y / anchor.w, 0.0f));
        }

        text_renderer_->render_text(
            *v,
            m1,
            m2,
            display_settings_->text_colour,
            1.0 / 1920.0,
            display_settings_->font_size,
            display_settings_->text_opacity);
    }
}

void MediaMetadataRenderer::init_overlay_opengl() {

    text_renderer_ = std::make_unique<ui::opengl::OpenGLTextRendererSDF>(
        utility::xstudio_resources_dir("fonts/VeraMono.ttf"), 96);

    // Set up the geometry used at draw time ... it couldn't be more simple,
    // it's just two triangles to make a rectangle
    glGenBuffers(1, &vbo_);
    glGenVertexArrays(1, &vao_);

    static std::array<float, 24> vertices = {
        // 1st triangle
        -1.0f,
        1.0f,
        0.0f,
        1.0f, // top left
        1.0f,
        1.0f,
        0.0f,
        1.0f, // top right
        1.0f,
        -1.0f,
        0.0f,
        1.0f, // bottom right
        // 2nd triangle
        1.0f,
        -1.0f,
        0.0f,
        1.0f, // bottom right
        -1.0f,
        1.0f,
        0.0f,
        1.0f, // top left
        -1.0f,
        -1.0f,
        0.0f,
        1.0f // bottom left
    };

    glBindVertexArray(vao_);
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
    // 3. then set our vertex module pointers
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    shader_ = std::make_unique<ui::opengl::GLShaderProgram>(vertex_shader, frag_shader);
}
