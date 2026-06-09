// SPDX-License-Identifier: Apache-2.0

#include "xstudio/ui/opengl/shader_program_base.hpp"
#include "xstudio/ui/opengl/opengl_multi_buffered_texture.hpp"
#include "wipe_viewport_layout_renderer.hpp"

using namespace xstudio::ui::viewport;
using namespace xstudio::ui::opengl;

ViewportWipeRenderer::ViewportWipeRenderer(const std::string &window_id, const utility::JsonStore &prefs)
    : OpenGLViewportRenderer(window_id, prefs) {}

ViewportWipeRenderer::~ViewportWipeRenderer() {
    if (wipe_vbo_)
        glDeleteBuffers(1, &wipe_vbo_);
    if (wipe_vao_)
        glDeleteVertexArrays(1, &wipe_vao_);
}

void ViewportWipeRenderer::pre_init() {
    OpenGLViewportRenderer::pre_init();
    glGenBuffers(1, &wipe_vbo_);
    glGenVertexArrays(1, &wipe_vao_);
}

void ViewportWipeRenderer::draw_image(
    const media_reader::ImageBufPtr &image_to_be_drawn,
    const media_reader::ImageSetLayoutDataPtr &layout_data,
    const int index,
    const Imath::M44f &window_to_viewport_matrix,
    const Imath::M44f &viewport_to_image_space,
    const float viewport_du_dx) {

    // wipe value is the position of the wipe handle normalised to the
    // viewport width.
    float wipe_screen_space = 0.5f;
    if (layout_data && layout_data->custom_layout_data_.contains("wipe_pos")) {
        wipe_screen_space = layout_data->custom_layout_data_.value("wipe_pos", 0.5f);
    }

    const bool first_image = layout_data->image_draw_order_hint_.size() > 1 &&
                                     index == layout_data->image_draw_order_hint_[1]
                                 ? false
                                 : true;

    if (wipe_screen_space <= 0.011f && first_image) {
        // wipe at the far left of the screen. Don't draw the wipe
        return;
    } else if (wipe_screen_space > 0.989f && !first_image) {
        // wipe at the far right of the screen. Don't draw the wipe
        return;
    }

    // re-normalise to -1.0,1.0 and multiply by projection matrix to get wipe
    // position in image space
    Imath::V4f w(-1.0f + wipe_screen_space * 2.0f, 0.0f, 0.0f, 1.0f);
    w *= viewport_to_image_space;
    float wipe_pos_image_space = w.x / w.w;

    if (wipe_pos_image_space <= -1.0f && first_image) {
        // wipe is all the way left. Only draw second image!
        return;
    } else if (wipe_pos_image_space > 0.9999f && !first_image) {
        // wipe is all the way right. Don't draw second image!
        return;
    }

    const bool no_wipe = layout_data->image_draw_order_hint_.size() < 2 ||
                         (wipe_screen_space <= 0.011f || wipe_screen_space > 0.989f) ||
                         (wipe_pos_image_space <= -1.0f || wipe_pos_image_space > 0.9999f);


    active_shader_program_->use();

    // set-up core shader parameters (e.g. image transform matrix etc)
    init_shader_uniforms(
        image_to_be_drawn,
        window_to_viewport_matrix,
        viewport_to_image_space,
        viewport_du_dx,
        layout_data->custom_layout_data_,
        index);

    {

        float left  = no_wipe ? -1.0f : first_image ? -1.0 : wipe_pos_image_space;
        float right = no_wipe ? 1.0f : first_image ? wipe_pos_image_space : 1.0;
        std::array<float, 24> vertices = {
            // 1st triangle
            left,
            1.0f,
            0.0f,
            1.0f, // top left
            right,
            1.0f,
            0.0f,
            1.0f, // top right
            right,
            -1.0f,
            0.0f,
            1.0f, // bottom right
            // 2nd triangle
            right,
            -1.0f,
            0.0f,
            1.0f, // bottom right
            left,
            1.0f,
            0.0f,
            1.0f, // top left
            left,
            -1.0f,
            0.0f,
            1.0f // bottom left
        };

        glBindVertexArray(wipe_vao_);
        // 2. copy our vertices array in a buffer for OpenGL to use
        glBindBuffer(GL_ARRAY_BUFFER, wipe_vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
        // 3. then set our vertex module pointers
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDisable(GL_BLEND);

        // the actual draw!
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(0);
        glBindVertexArray(0);
    }


    glUseProgram(0);
}
