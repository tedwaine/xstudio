// SPDX-License-Identifier: Apache-2.0
#include "audio_waveform_overlay_renderer.hpp"

using namespace xstudio;
using namespace xstudio::ui::viewport;

namespace {
const char *vertex_shader = R"(
    #version 330 core
    layout (location = 0) in float ypos;
    uniform mat4 to_coord_system;
    uniform mat4 to_canvas;
    uniform float hscale;
    uniform float vscale;
    uniform float v_pos;
    uniform float du_dx;
    uniform float horiz_scale;
    uniform int offset;
    uniform int red_line;
   // uniform int x_nudge;
    //uniform int y_nudge;
    flat out int inside_current_frame;

    void main()
    {
        float uvv = float(gl_VertexID-offset)*hscale*horiz_scale*0.5 - (horiz_scale-1.0)/2.0f;
        inside_current_frame = int(uvv > 0.0 && uvv < 1.0);
        vec4 rpos = vec4(-1.0 + float(gl_VertexID-offset)*hscale, v_pos+ypos*vscale*10.0, vec2(0.0, 1.0));
        //rpos.x += (-0.5f + (x_nudge + 0.5f) / 4.0f)*du_dx*2.0;
        //rpos.y += (-0.5f + (y_nudge + 0.5f) / 4.0f)*du_dx*2.0;
        gl_Position = rpos*to_canvas;
    }
    )";

const char *frag_shader = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 line_colour;
    uniform vec3 extra_line_colour;
    
    flat in int inside_current_frame;

    void main(void)
    {
        FragColor = vec4(inside_current_frame==1 ? line_colour : extra_line_colour, 1.0);
    }

    )";
} // namespace

AudioWaveformOverlayRenderer::~AudioWaveformOverlayRenderer() {
    if (vbo_)
        glDeleteBuffers(1, &vbo_);
    if (vao_)
        glDeleteBuffers(1, &vao_);
}

void AudioWaveformOverlayRenderer::render_image_overlay(
    const Imath::M44f &transform_window_to_viewport_space,
    const Imath::M44f &transform_viewport_to_image_space,
    const float viewport_du_dpixel,
    const float device_pixel_ratio,
    const xstudio::media_reader::ImageBufPtr &frame) {

    if (!shader_)
        init_overlay_opengl();

    auto render_data =
        frame.plugin_blind_data(utility::Uuid("873c508b-276b-44e3-82d0-15db2f039aa7"));
    if (!render_data)
        return;

    const auto *data = dynamic_cast<const WaveFormData *>(render_data.get());
    if (!data)
        return;

    glBindVertexArray(vao_);
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(
        GL_ARRAY_BUFFER,
        data->verts_.size() * sizeof(float),
        data->verts_.data(),
        GL_STREAM_DRAW);
    // 3. then set our vertex module pointers
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    const int n_samps = data->verts_.size() / data->num_chans;

    utility::JsonStore shader_params;
    shader_params["to_canvas"]         = transform_window_to_viewport_space;
    shader_params["hscale"]            = 2.0f / float(n_samps);
    shader_params["vscale"]            = data->vscale * device_pixel_ratio;
    shader_params["line_colour"]       = data->line_colour;
    shader_params["extra_line_colour"] = data->extra_line_colour;
    shader_params["du_dx"]             = viewport_du_dpixel;
    shader_params["horiz_scale"]       = data->horizontal_scale;
    shader_->set_shader_parameters(shader_params);
    shader_->use();
    glEnableVertexAttribArray(0);
    glDisable(GL_BLEND);
    utility::JsonStore es;

    for (int c = 0; c < data->num_chans; ++c) {

        glLineWidth(1.0f);
        es["v_pos"]    = data->v_pos + data->chan_spacing * c;
        es["offset"]   = c * n_samps;
        es["red_line"] = 0;
        shader_->set_shader_parameters(es);

        // the actual draw! (dodgy anti-aliasing commented out)
        /*for (int x_nudge = 0; x_nudge < 4; x_nudge++) {
            v["x_nudge"] = x_nudge;
            for (int y_nudge = 0; y_nudge < 4; y_nudge++) {
                v["y_nudge"] = y_nudge;
                shader_->set_shader_parameters(v);
                glDrawArrays(GL_LINE_STRIP, c * n_samps, n_samps);
            }
        }*/
        glDrawArrays(GL_LINE_STRIP, c * n_samps, n_samps);

        es["line_colour"] = utility::ColourTriplet(1.0, 0.0, 0.0);
        es["red_line"]    = 1;
        shader_->set_shader_parameters(es);
        glLineWidth(3.0f);
        glDrawArrays(GL_LINE_STRIP, 0, 2);
    }
    shader_->stop_using();
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
}

void AudioWaveformOverlayRenderer::init_overlay_opengl() {

    glGenBuffers(1, &vbo_);
    glGenVertexArrays(1, &vao_);

    shader_ = std::make_unique<ui::opengl::GLShaderProgram>(vertex_shader, frag_shader);
}