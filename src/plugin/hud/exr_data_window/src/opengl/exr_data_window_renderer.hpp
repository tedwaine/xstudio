#pragma once

#ifdef __apple__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#include "xstudio/plugin_manager/plugin_base.hpp"
#include "xstudio/ui/opengl/shader_program_base.hpp"

class EXRDataWindowRenderer : public plugin::ViewportOverlayRenderer {

  public:
    Imath::V2f get_transformed_point(
        const Imath::V2i &point, const Imath::V2i &image_dims, const float pixel_aspect) {
        const float aspect = float(image_dims.y) / float(image_dims.x);

        float norm_x = float(point.x) / image_dims.x;
        float norm_y = float(point.y) / image_dims.y;

        return Imath::V2f(norm_x * 2.0f - 1.0f, (norm_y * 2.0f - 1.0f) * aspect / pixel_aspect);
    };

    void render_image_overlay(
        const Imath::M44f &transform_window_to_viewport_space,
        const Imath::M44f &transform_viewport_to_image_space,
        const float /*viewport_du_dpixel*/,
        const float /*device_pixel_ratio*/,
        const xstudio::media_reader::ImageBufPtr &frame) override {

        utility::BlindDataObjectPtr render_data =
            frame.plugin_blind_data(utility::Uuid("f8a09960-606d-11ed-9b6a-0242ac120002"));
        const auto *data = dynamic_cast<const HudData *>(render_data.get());
        if (data && frame) {

            if (!shader_)
                init_overlay_opengl();

            auto image_dims = frame ? frame->image_size_in_pixels() : Imath::V2i(0);
            auto image_bounds_min =
                frame ? frame->image_pixels_bounding_box().min : Imath::V2i(0);
            auto image_bounds_max =
                frame ? frame->image_pixels_bounding_box().max : Imath::V2i(0);

            Imath::V2f top_left = get_transformed_point(
                image_bounds_min, image_dims, frame.frame_id().pixel_aspect());
            Imath::V2f bottom_right = get_transformed_point(
                image_bounds_max, image_dims, frame.frame_id().pixel_aspect());

            // NOLINT
            std::array<float, 16> vertices = {
                top_left.x,
                top_left.y,
                0.0f,
                1.0f,
                top_left.x,
                bottom_right.y,
                0.0f,
                1.0f,
                bottom_right.x,
                bottom_right.y,
                0.0f,
                1.0f,
                bottom_right.x,
                top_left.y,
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

    void init_overlay_opengl() {

        glGenBuffers(1, &vertex_buffer_object_);
        glGenVertexArrays(1, &vertex_array_object_);
        shader_ = std::make_unique<ui::opengl::GLShaderProgram>(vertex_shader, frag_shader);
    }

    std::unique_ptr<xstudio::ui::opengl::GLShaderProgram> shader_;
    GLuint vertex_buffer_object_;
    GLuint vertex_array_object_;
};
