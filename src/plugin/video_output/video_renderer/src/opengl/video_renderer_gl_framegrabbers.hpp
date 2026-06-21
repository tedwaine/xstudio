// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "xstudio/ui/opengl/opengl_viewport_renderer.hpp"
#include "xstudio/ui/opengl/opengl_offscreen_renderer.hpp"
#include "xstudio/ui/opengl/shader_program_base.hpp"

/*
NOTE: These classes are currently UNUSED! Leaving here just in case they are
needed in the future.
*/

namespace xstudio::ui::viewport {

    class YUVFrameGrabber : public ui::viewport::ViewportFramePostProcessor {

    public:
        YUVFrameGrabber() {}
        ~YUVFrameGrabber();

        void viewport_capture_framebuffer(
            uint32_t tex_id,
            uint32_t fbo_id,
            const int fb_width,
            const int fb_height,
            const ui::viewport::ImageFormat format,
            media_reader::ImageBufPtr &destination_image) override;

    private:
        void setupYUVImageTextureAndFrameBuffer(const int width, const int __height);

        void grabYUVFrameBuffer(
            const int width, const int height, media_reader::ImageBufPtr &destination_image);

        GLuint pixel_buffer_object_ = 0;
        int pix_buf_size_           = 0;
        GLuint texId_               = 0;
        GLuint fboId_               = 0;
        GLuint vbo_                 = 0;
        GLuint vao_                 = 0;
        int tex_width_              = 0;
        int tex_height_             = 0;
        std::unique_ptr<ui::opengl::GLShaderProgram> shader_;
    };

    class RGB10BitFrameGrabber : public ui::viewport::ViewportFramePostProcessor {

    public:
        RGB10BitFrameGrabber() {}
        ~RGB10BitFrameGrabber();

        void viewport_capture_framebuffer(
            uint32_t tex_id,
            uint32_t fbo_id,
            const int fb_width,
            const int fb_height,
            const ui::viewport::ImageFormat format,
            media_reader::ImageBufPtr &destination_image) override;

    private:
        void setupImageTextureAndFrameBuffer(const int width, const int height);

        void grabRGB10bitFrameBuffer(
            const int width, const int height, media_reader::ImageBufPtr &destination_image);

        GLuint pixel_buffer_object_ = 0;
        int pix_buf_size_           = 0;
        GLuint texId_               = 0;
        GLuint fboId_               = 0;
        GLuint vbo_                 = 0;
        GLuint vao_                 = 0;
        int tex_width_              = 0;
        int tex_height_             = 0;
        std::unique_ptr<ui::opengl::GLShaderProgram> shader_;
    };
} // namespace xstudio::ui::viewport