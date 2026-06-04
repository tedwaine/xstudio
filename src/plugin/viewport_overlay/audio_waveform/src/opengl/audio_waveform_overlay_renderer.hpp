// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "../audio_waveform_overlay.hpp"

#ifdef __apple__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif

namespace xstudio::ui::viewport {

class AudioWaveformOverlayRenderer : public plugin::ViewportOverlayRenderer {

  public:
    void render_image_overlay(
        const Imath::M44f &transform_window_to_viewport_space,
        const Imath::M44f &transform_viewport_to_image_space,
        const float viewport_du_dpixel,
        const float device_pixel_ratio,
        const xstudio::media_reader::ImageBufPtr &frame) override;

    ~AudioWaveformOverlayRenderer();

    void init_overlay_opengl();

    std::unique_ptr<xstudio::ui::opengl::GLShaderProgram> shader_;
    GLuint vbo_ = {0};
    GLuint vao_ = {0};
};

} // namespace xstudio::ui::viewport
