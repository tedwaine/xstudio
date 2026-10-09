// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "xstudio/colour_pipeline/colour_pipeline.hpp"
#include "xstudio/media_reader/image_buffer_set.hpp"
#include "xstudio/ui/viewport/viewport.hpp"
#include "xstudio/utility/uuid.hpp"
#include <Imath/ImathMatrix.h>
#include <caf/actor.hpp>

namespace xstudio::ui::metal {

// Note we can't include "metal_shader_program.hpp" here because it is Objective-C++ and this header 
// is included in C++ only files elsewhere.
class MetalShaderProgram;
typedef std::shared_ptr<MetalShaderProgram> MetalShaderProgramPtr;
class MetalRenderPipe;
typedef std::shared_ptr<MetalRenderPipe> MetalRenderPipePtr;

struct MetalRendererInterface : public viewport::RendererInterface {
    void *command_encoder  = nullptr;
    void *device  = nullptr;
    int framesInFlight = 0;
    int currentFrameSlot = 0;
};

class ColourPipeLutCollection {
  public:
    ColourPipeLutCollection() = default;
    ColourPipeLutCollection(const ColourPipeLutCollection &o) = default;

    void upload_luts(const std::vector<colour_pipeline::ColourLUTPtr> &luts) {}
    void register_texture(const std::vector<colour_pipeline::ColourTexture> &textures) {}

    // void bind_luts(GLShaderProgramPtr shader, int &tex_idx);

    void clear() { } //active_luts_.clear(); }

  private:
    /*typedef std::shared_ptr<GLColourLutTexture> GLColourLutTexturePtr;
    std::map<std::string, GLColourLutTexturePtr> lut_textures_;
    std::vector<GLColourLutTexturePtr> active_luts_;*/
    std::map<std::string, colour_pipeline::ColourTexture> active_textures_;
};

class MetalViewportRenderer : public viewport::ViewportRenderer {
  public:
    MetalViewportRenderer(const std::string &window_id, const utility::JsonStore &prefs);

    ~MetalViewportRenderer() override;

    void render(
        viewport::RendererInterfacePtr &renderer_interface,
        const media_reader::ImageBufDisplaySetPtr &images,
        const Imath::M44f &window_to_viewport_matrix,
        const Imath::M44f &viewport_to_image_matrix,
        const Imath::V2i &window_size,
        const float device_pixel_ratio,
        const std::vector<plugin::ViewportOverlayRendererPtr> &overlay_renderers) override;

    void set_depth(const float depth) override;

    void store_client_state() override;

    void restore_client_state() override;

    virtual void draw_image(
        viewport::RendererInterfacePtr &renderer_interface,
        const media_reader::ImageBufPtr &image_to_be_drawn,
        const media_reader::ImageSetLayoutDataPtr &layout_data,
        const int index,
        const Imath::M44f &window_to_viewport_matrix,
        const Imath::M44f &viewport_to_image_space,
        const float viewport_du_dx);

  protected:
    /**
     * @brief Initialise the active shader uniforms
     */
    virtual void init_shader_uniforms(
        const media_reader::ImageBufPtr &image_to_be_drawn,
        const Imath::M44f &window_to_viewport_matrix,
        const Imath::M44f &viewport_to_image_space,
        const float viewport_du_dx,
        const utility::JsonStore &layout_data,
        const int image_index) const;

    const std::array<int, 4> &viewport_coords_in_window() { return viewport_coords_in_window_; }

    void __draw_image(
    viewport::RendererInterfacePtr &renderer_interface,
        const media_reader::ImageBufDisplaySetPtr &all_images,
        const int index,
        const Imath::M44f &window_to_viewport_matrix,
        const Imath::M44f &viewport_to_image_space,
        const float viewport_du_dx);

    void __draw_per_image_overlays(
        viewport::RendererInterfacePtr &renderer_interface,
        const media_reader::ImageBufDisplaySetPtr &all_images,
        const int index,
        const Imath::M44f &window_to_viewport_matrix,
        const Imath::M44f &viewport_to_image_space,
        const float viewport_du_dx,
        const float device_pixel_ratio,
        const std::vector<plugin::ViewportOverlayRendererPtr> &overlay_renderers);

    void pre_init(viewport::RendererInterfacePtr &renderer_interface) override {}

    void do_init(viewport::RendererInterfacePtr &renderer_interface);

    bool activate_shader(
        const viewport::GPUShaderPtr &image_buffer_unpack_shader,
        const std::vector<colour_pipeline::ColourOperationDataPtr> &operations);

    void upload_image_and_colour_data(const media_reader::ImageBufPtr &image);
    void bind_textures(const media_reader::ImageBufPtr &image);
    void release_textures();
    void clear_viewport_area(
        viewport::RendererInterfacePtr &renderer_interface,
        const Imath::M44f &to_scene_matrix,
        const Imath::V2i &window_size
    );

    std::string latest_colour_pipe_data_cacheid_;
    const std::string window_id_;
    std::array<int, 4> viewport_coords_in_window_;
    float clear_alpha_ = {1.0f};
    MetalShaderProgramPtr active_shader_program_;
    MetalShaderProgramPtr no_image_shader_program_;
    ColourPipeLutCollection colour_pipe_lut_collection_;
    std::map<std::string, MetalShaderProgramPtr> shader_programs_;
    MetalRenderPipePtr render_pipe_;
};
} // namespace xstudio::ui::metal
