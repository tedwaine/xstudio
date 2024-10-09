// SPDX-License-Identifier: Apache-2.0
#pragma once

// clang-format off
#include <GL/glew.h>
#include <GL/gl.h>
// clang-format on

#include "xstudio/media_reader/media_reader.hpp"
#include "xstudio/colour_pipeline/colour_pipeline.hpp"
#include "xstudio/utility/uuid.hpp"

//#define USE_SSBO

namespace xstudio {
namespace ui {
    namespace opengl {

        class GLBlindTex {

          public:
            GLBlindTex() = default;
            ~GLBlindTex();

            void release();

            virtual void bind(int tex_index, Imath::V2i &dims) {
                wait_on_upload_pixels();
                __bind(tex_index, dims);
            }

            [[nodiscard]] const media::MediaKey &media_key() const { return media_key_; }
            [[nodiscard]] const utility::time_point &when_last_used() const {
                return when_last_used_;
            }

            void upload_image_buffer(media_reader::ImageBufPtr &frame);

          protected:
            virtual uint8_t *map_buffer_for_upload()             = 0;
            virtual void __bind(int tex_index, Imath::V2i &dims) = 0;
            virtual size_t tex_size_bytes() const                = 0;

            void wait_on_upload_pixels() {
                std::unique_lock lk(mutex_);
                if (uploading_pixels_)
                    cv_.wait(lk, [=]() { return !uploading_pixels_; });
            }

            void do_pixel_upload(uint8_t *target, uint8_t *src, size_t n, media::MediaKey k);

            utility::time_point when_last_used_;

            media::MediaKey media_key_;
            media_reader::ImageBufPtr source_frame_;

            std::thread upload_thread_;
            std::mutex mutex_;
            std::condition_variable cv_;
            bool uploading_pixels_ = {false};
        };

        class GLSsboTex : public GLBlindTex {

          public:
            GLSsboTex() = default;
            virtual ~GLSsboTex();

            uint8_t *map_buffer_for_upload() override;
            void __bind(int /*tex_index*/, Imath::V2i & /*dims*/) override;

          private:
            void compute_size(const size_t required_size_bytes);
            void pixel_upload();
            void wait_on_upload();

            GLuint ssbo_id_         = {0};
            GLuint bytes_per_pixel_ = 4;

            [[nodiscard]] size_t tex_size_bytes() const override { return tex_data_size_; }

            size_t tex_data_size_ = {0};
        };

        class GLBlindRGBA8bitTex : public GLBlindTex {

          public:
            GLBlindRGBA8bitTex() = default;
            virtual ~GLBlindRGBA8bitTex();

            uint8_t *map_buffer_for_upload() override;
            void __bind(int tex_index, Imath::V2i &dims) override;

          private:
            void resize(const size_t required_size_bytes);
            void pixel_upload();

            [[nodiscard]] size_t tex_size_bytes() const override {
                return tex_width_ * tex_height_ * bytes_per_pixel_;
            }

            GLuint bytes_per_pixel_     = {0};
            GLuint tex_id_              = {0};
            GLuint pixel_buf_object_id_ = {0};

            int tex_width_  = {0};
            int tex_height_ = {0};
        };


        class GLDoubleBufferedTexture {

          public:
            GLDoubleBufferedTexture();
            virtual ~GLDoubleBufferedTexture() = default;

            void bind(int &tex_index, Imath::V2i &dims);
            void release();

            void upload_next(std::vector<media_reader::ImageBufPtr>);

            void set_use_ssbo(const bool using_ssbo);

          private:
            typedef std::shared_ptr<GLBlindTex> GLBlindTexturePtr;
            GLBlindTexturePtr current_;
            std::vector<GLBlindTexturePtr> textures_;
            media::MediaKey active_media_key_;
            bool using_ssbo_ = {false};
        };


        class GLColourLutTexture {

          public:
            GLColourLutTexture(
                const colour_pipeline::LUTDescriptor desc, const std::string texture_name);
            virtual ~GLColourLutTexture();

            void bind(int tex_index);
            void release();
            void upload_texture_data(const colour_pipeline::ColourLUTPtr &lut);

            [[nodiscard]] GLuint texture_id() const { return tex_id_; }
            [[nodiscard]] GLenum target() const;
            [[nodiscard]] const std::string &texture_name() const { return texture_name_; }

          private:
            GLint interpolation();
            GLint internal_format();
            GLint data_type();
            GLint format();

            GLuint tex_id_ = {0};
            GLuint pbo_    = {0};
            const colour_pipeline::LUTDescriptor descriptor_;
            std::size_t lut_cache_id_;
            const std::string texture_name_;
        };
    } // namespace opengl
} // namespace ui
} // namespace xstudio