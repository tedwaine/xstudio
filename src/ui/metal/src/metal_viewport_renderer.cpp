// SPDX-License-Identifier: Apache-2.0

#include "xstudio/ui/metal/metal_viewport_renderer.hpp"
#include "xstudio/media_reader/media_reader.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/utility/uuid.hpp"

using namespace xstudio;
using namespace xstudio::ui::viewport;
using namespace xstudio::ui::metal;
using namespace xstudio::media_reader;
using namespace xstudio::colour_pipeline;
using namespace xstudio::utility;

MetalViewportRenderer::MetalViewportRenderer(
    const std::string &window_id, const utility::JsonStore &prefs)
    : viewport::ViewportRenderer(), window_id_(window_id) {

    // Instances of MetalViewportRenderer that are in the same xstudio
    // window need to share texture resources as they display exactly the
    // same image.

}

MetalViewportRenderer::~MetalViewportRenderer() {

}


void MetalViewportRenderer::upload_image_and_colour_data(
    const media_reader::ImageBufPtr &image) {


}

void MetalViewportRenderer::bind_textures(const media_reader::ImageBufPtr &image) {

}

void MetalViewportRenderer::release_textures() {

}

// static std::mutex m;

void MetalViewportRenderer::clear_viewport_area(
    const Imath::M44f &window_to_viewport_matrix, const Imath::V2i &window_size) {

}

void MetalViewportRenderer::store_client_state() {
}


void MetalViewportRenderer::restore_client_state() {
}

void MetalViewportRenderer::set_depth(const float depth) {

}


void MetalViewportRenderer::render(
    const media_reader::ImageBufDisplaySetPtr &images,
    const Imath::M44f &window_to_viewport_matrix,
    const Imath::M44f &viewport_to_image_space,
    const Imath::V2i &window_size,
    const float device_pixel_ratio,
    const std::vector<plugin::ViewportOverlayRendererPtr> &overlay_renderers) {


}

void MetalViewportRenderer::__draw_image(
    const media_reader::ImageBufDisplaySetPtr &images,
    const int index,
    const Imath::M44f &window_to_viewport_matrix,
    const Imath::M44f &viewport_to_image_space,
    const float viewport_du_dx) {

    if (!images || images->empty()) {
        return;
    }

    media_reader::ImageBufPtr image_to_be_drawn = images->onscreen_image(index);
    if (!image_to_be_drawn)
        return;

    Imath::M44f to_image_matrix =
        (image_to_be_drawn.layout_transform() * viewport_to_image_space.inverse()).inverse();

    /* Here we allow plugins to run arbitrary GPU draw & computation routines.
    This will allow pixel data to be rendered to textures (offscreen), for example,
    which can then be sampled at actual draw time.*/
    for (auto hook : pre_render_gpu_hooks_) {
        hook.second->pre_viewport_draw_gpu_hook(
            window_to_viewport_matrix, to_image_matrix, viewport_du_dx, image_to_be_drawn);
    }

    if (image_to_be_drawn.invisible())
        return;

    // if we've received a new image and/or colour pipeline data (LUTs etc) since the last
    // draw, upload the data
    //upload_image_and_colour_data(image_to_be_drawn);

    draw_image(
        image_to_be_drawn,
        images->layout_data(),
        index,
        window_to_viewport_matrix,
        viewport_to_image_space,
        viewport_du_dx);

    //textures()[0]->release(image_to_be_drawn);
}


void MetalViewportRenderer::__draw_per_image_overlays(
    const media_reader::ImageBufDisplaySetPtr &images,
    const int index,
    const Imath::M44f &window_to_viewport_matrix,
    const Imath::M44f &viewport_to_image_space,
    const float viewport_du_dx,
    const float device_pixel_ratio,
    const std::vector<plugin::ViewportOverlayRendererPtr> &overlay_renderers) {


    if (!images || images->empty()) {
        return;
    }

    media_reader::ImageBufPtr target_image = images->onscreen_image(index);

    Imath::M44f to_image_matrix =
        (target_image.layout_transform() * viewport_to_image_space.inverse()).inverse();

    /* Call the render functions of overlay plugins - note that if the overlay prefers to draw
    before the image but we have no alpha channel, we still call its render function here */
    /*if (target_image) {

        for (auto &orf : overlay_renderers) {
            orf->render_image_overlay(
                window_to_viewport_matrix,
                to_image_matrix,
                abs(viewport_du_dx),
                device_pixel_ratio,
                target_image);
        }

        // display err message attached to image if there is one
        if (!target_image.error_details().empty()) {

            std::vector<float> vtxs;
            std::ignore = resources_->text_renderer_->precompute_text_rendering_vertex_layout(
                vtxs,
                target_image.error_details(),
                Imath::V2f(0.0f, 0.0f),
                1.0f,
                24.0f,
                JustifyCentre,
                1.0f);

            resources_->text_renderer_->render_text(
                vtxs,
                window_to_viewport_matrix,
                to_image_matrix,
                utility::ColourTriplet(1.0f, 1.0f, 1.0f),
                viewport_du_dx,
                15.0f,
                1.0f);
        }
    }*/
}

void MetalViewportRenderer::draw_image(
    const media_reader::ImageBufPtr &image_to_be_drawn,
    const media_reader::ImageSetLayoutDataPtr &layout_data,
    const int index,
    const Imath::M44f &window_to_viewport_matrix,
    const Imath::M44f &viewport_to_image_space,
    const float viewport_du_dx) {


    /*active_shader_program_->use();

    // set-up core shader parameters (e.g. image transform matrix etc)
    init_shader_uniforms(
        image_to_be_drawn,
        window_to_viewport_matrix,
        viewport_to_image_space,
        viewport_du_dx,
        layout_data->custom_layout_data_,
        index);

    glDisable(GL_BLEND);
    // the actual draw .. a quad that spans -1.0, 1.0 in x & y.
    glBindVertexArray(vao());
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
    glUseProgram(0);*/
}

bool MetalViewportRenderer::activate_shader(
    const viewport::GPUShaderPtr &virt_image_buffer_unpack_shader,
    const std::vector<colour_pipeline::ColourOperationDataPtr> &colour_operations) {

    return false;

    /*if (!virt_image_buffer_unpack_shader ||
        virt_image_buffer_unpack_shader->graphics_api() != GraphicsAPI::OpenGL) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, "No shader passed with image buffer.");
        return false;
    }

    auto image_buffer_unpack_shader =
        static_cast<opengl::OpenGLShader const *>(virt_image_buffer_unpack_shader.get());
    if (!image_buffer_unpack_shader) {
    }

    std::cerr << "FOPKL " << image_buffer_unpack_shader << " " << image_buffer_unpack_shader->shader_code() << std::endl;
    std::string shader_id = to_string(image_buffer_unpack_shader->shader_id());
std::cerr << "Fasfa " << shader_id << "\n";

    for (const auto &op : colour_operations) {
        shader_id += op->cache_id();
    }

    // do we already have this shader compiled?
    if (shader_programs().find(shader_id) == shader_programs().end()) {

        // try to compile the shader for this combo of image buffer unpack
        // and colour pipeline components

        try {

            std::vector<std::string> shader_components;
            for (const auto &colour_op : colour_operations) {
                // sanity check - this should be impossible, though
                if (colour_op->shader_->graphics_api() != GraphicsAPI::OpenGL) {
                    throw std::runtime_error(
                        "Non-OpenGL shader data in colour operation chain!");
                }
                auto pr = static_cast<opengl::OpenGLShader const *>(colour_op->shader_.get());
                shader_components.push_back(pr->shader_code());
            }

            shader_programs()[shader_id].reset(new GLShaderProgram(
                default_vertex_shader,
                image_buffer_unpack_shader->shader_code(),
                shader_components,
                use_ssbo_));

        } catch (std::exception &e) {
            spdlog::error("{}", e.what());
            shader_programs()[shader_id].reset();
        }
    }

    if (shader_programs()[shader_id]) {
        active_shader_program_ = shader_programs()[shader_id];
    } else {
        active_shader_program_ = no_image_shader_program();
    }

    return active_shader_program_ != no_image_shader_program();*/
}

void MetalViewportRenderer::init_shader_uniforms(
    const media_reader::ImageBufPtr &image_to_be_drawn,
    const Imath::M44f &window_to_viewport_matrix,
    const Imath::M44f &viewport_to_image_space,
    const float viewport_du_dx,
    const utility::JsonStore &layout_data,
    const int image_index) const {
    
    /*   try {
        // set-up core shader parameters (e.g. image transform matrix etc)
        utility::JsonStore shader_params = core_shader_params(
            image_to_be_drawn,
            window_to_viewport_matrix,
            viewport_to_image_space,
            viewport_du_dx,
            layout_data,
            image_index);

        active_shader_program_->set_shader_parameters(shader_params);

        if (image_to_be_drawn) {
            active_shader_program_->set_shader_parameters(image_to_be_drawn);
            active_shader_program_->set_shader_parameters(
                image_to_be_drawn.colour_pipe_uniforms());
        }
    } catch (std::exception &e) {
        spdlog::error("{} {}", __PRETTY_FUNCTION__, e.what());
    }*/ 
}

void MetalViewportRenderer::pre_init() { 
    // resources_->init(); 
}

