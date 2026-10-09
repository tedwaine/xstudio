// SPDX-License-Identifier: Apache-2.0

#include <Metal/Metal.h>
#include <AppKit/AppKit.h>

#include "xstudio/ui/metal/metal_viewport_renderer.hpp"
#include "xstudio/ui/metal/metal_shader_program.hpp"
#include "xstudio/ui/metal/no_image_shader_program.hpp"
#include "xstudio/ui/metal/shader_program_base.hpp"
#include "xstudio/media_reader/media_reader.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/utility/uuid.hpp"

using namespace xstudio;
using namespace xstudio::ui::viewport;
using namespace xstudio::ui::metal;
using namespace xstudio::media_reader;
using namespace xstudio::colour_pipeline;
using namespace xstudio::utility;

namespace xstudio::ui::metal {

class MetalRenderPipe {
public:
    id<MTLDevice> device_;
    id<MTLBuffer> vbuf_ ;
    id<MTLBuffer> ubuf_[3];
    id<MTLRenderPipelineState> pipeline_;
    MTLRenderPipelineDescriptor *rpDesc = nullptr;
    MTLRenderPipelineReflection *reflection = nullptr;
};

} // namespace xstudio::ui::metal

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

    colour_pipeline::ColourPipelineDataPtr colour_pipe_data = image.colour_pipe_data();

    /*if (!textures().size())
        return;*/

    if (image) {

        if (image->error_state() == BufferErrorState::HAS_ERROR) {
            // the frame contains errors, no need to continue from that point
            active_shader_program_ = no_image_shader_program_;
            return;
        }

        // check if the frame we need to draw has already been
        // uploaded to texture memory and set the 'draw_texture_index_'
        // accordingly
        // textures()[0]->upload_image(image);
    }

    if (colour_pipe_data && colour_pipe_data->cache_id() != latest_colour_pipe_data_cacheid_) {
        colour_pipe_lut_collection_.clear();
        for (const auto &op : colour_pipe_data->operations()) {
            colour_pipe_lut_collection_.upload_luts(op->luts_);
            colour_pipe_lut_collection_.register_texture(op->textures_);
        }
        latest_colour_pipe_data_cacheid_ = colour_pipe_data->cache_id();
    }

    if (!(image && colour_pipe_data &&
          activate_shader(image->shader(), colour_pipe_data->operations()))) {

        active_shader_program_ = no_image_shader_program_;
    }

    bind_textures(image);

}

void MetalViewportRenderer::bind_textures(const media_reader::ImageBufPtr &image) {

}

void MetalViewportRenderer::release_textures() {

}

// static std::mutex m;

void MetalViewportRenderer::clear_viewport_area(
    viewport::RendererInterfacePtr &renderer_interface,
    const Imath::M44f &window_to_viewport_matrix, const Imath::V2i &window_size) {

    MetalRendererInterface *stateInfo = static_cast<MetalRendererInterface *>(renderer_interface.get());
    id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>) stateInfo->command_encoder;
    
    MTLViewport vp;
    vp.originX = 0;
    vp.originY = 0;
    vp.width = window_size.x;
    vp.height = window_size.y;
    vp.znear = 0;
    vp.zfar = 1;
    [encoder setViewport: vp];

}

void MetalViewportRenderer::store_client_state() {
}


void MetalViewportRenderer::restore_client_state() {
}

void MetalViewportRenderer::set_depth(const float depth) {

}

void MetalViewportRenderer::render(
    viewport::RendererInterfacePtr &renderer_interface,
    const media_reader::ImageBufDisplaySetPtr &images,
    const Imath::M44f &window_to_viewport_matrix,
    const Imath::M44f &viewport_to_image_space,
    const Imath::V2i &window_size,
    const float device_pixel_ratio,
    const std::vector<plugin::ViewportOverlayRendererPtr> &overlay_renderers) {

std::cerr << "Render " << window_id_ << " " << window_size.x << " " << window_size.y << " " << renderer_interface << " " << this << "\n";
    if (!renderer_interface) {
        return;
    }

    MetalRendererInterface *stateInfo = static_cast<MetalRendererInterface *>(renderer_interface.get());

    id<MTLDevice> device_id = (__bridge id<MTLDevice>) stateInfo->device;

    if (!render_pipe_) {
        render_pipe_ = std::make_shared<MetalRenderPipe>();
    }

    if (device_id != render_pipe_->device_) {
        do_init(renderer_interface);
    }

    /*if (!renderer_) {
        renderer_ = new TestRenderer();
    }
    renderer_->render(stateInfo, window_size);*/

    // this value tells us how much we are zoomed into the image in the viewport (in
    // the x dimension). If the image is width-fitted exactly to the viewport, then this
    // value will be 1.0 (what it means is the coordinates -1.0 to 1.0 are mapped to
    // the width of the viewport)
    const float image_zoom_in_viewport = viewport_to_image_space[0][0];


    // this value gives us how much of the parent window is covered by the viewport.
    // So if the xstudio window is 1000px in width, and the viewport is 500px wide
    // (with the rest of the UI taking up the remainder) then this value will be 0.5
    const float viewport_x_size_in_window =
        window_to_viewport_matrix[0][0] / window_to_viewport_matrix[3][3];

    // this value tells us how much a screen pixel width in the viewport is in the units
    // of viewport coordinate space
    const float viewport_du_dx =
        image_zoom_in_viewport / (window_size.x * viewport_x_size_in_window);

    /* we do our own clear of the viewport */
    clear_viewport_area(renderer_interface, window_to_viewport_matrix, window_size);

    if (images && images->layout_data()) {

        // glDisable(GL_DEPTH_TEST);

        // we must avoid painting outside the geometry of the viewport window, or it will be
        // visible underneath QML elements with opactity etc. or other viewports in the
        // same window
        /*glScissor(
            viewport_coords_in_window()[0],
            viewport_coords_in_window()[1],
            viewport_coords_in_window()[2],
            viewport_coords_in_window()[3]);*/

        //textures()[0]->queue_image_set_for_upload(images);

        for (const auto &idx : images->layout_data()->image_draw_order_hint_) {
            __draw_image(
                renderer_interface,
                images,
                idx,
                window_to_viewport_matrix,
                viewport_to_image_space,
                viewport_du_dx);
        }

        // enable scissor
        if (images->layout_data()->draw_hero_overlays_only_) {
            __draw_per_image_overlays(
                renderer_interface,
                images,
                images->hero_sub_playhead_index(),
                window_to_viewport_matrix,
                viewport_to_image_space,
                viewport_du_dx,
                device_pixel_ratio,
                overlay_renderers);
        } else {
            for (const auto &idx : images->layout_data()->image_draw_order_hint_) {
                __draw_per_image_overlays(
                    renderer_interface,
                    images,
                    idx,
                    window_to_viewport_matrix,
                    viewport_to_image_space,
                    viewport_du_dx,
                    device_pixel_ratio,
                    overlay_renderers);
            }
        }
        // disable scissor
    }

    // enable scissor

    // Some plugins want to draw on the whole viewport canvas (not over a particular
    // image)
    for (auto orf : overlay_renderers) {

        orf->render_viewport_overlay(
            renderer_interface,
            window_to_viewport_matrix,
            viewport_to_image_space,
            images,
            abs(viewport_du_dx),
            device_pixel_ratio);
    }
    // disable scissor


#ifdef DEBUG_GRAB_FRAMEBUFFER
    grab_framebuffer_to_disk();
#endif

    // restore depth

}

void MetalViewportRenderer::__draw_image(
    viewport::RendererInterfacePtr &renderer_interface,
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
    upload_image_and_colour_data(image_to_be_drawn);

    draw_image(
        renderer_interface,
        image_to_be_drawn,
        images->layout_data(),
        index,
        window_to_viewport_matrix,
        viewport_to_image_space,
        viewport_du_dx);

    //textures()[0]->release(image_to_be_drawn);
}


void MetalViewportRenderer::__draw_per_image_overlays(
    viewport::RendererInterfacePtr &renderer_interface,
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
    if (target_image) {

        for (auto &orf : overlay_renderers) {
            orf->render_image_overlay(
                renderer_interface,
                window_to_viewport_matrix,
                to_image_matrix,
                abs(viewport_du_dx),
                device_pixel_ratio,
                target_image);
        }

        // display err message attached to image if there is one
        if (!target_image.error_details().empty()) {

            std::vector<float> vtxs;
            /*td::ignore = resources_->text_renderer_->precompute_text_rendering_vertex_layout(
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
                1.0f);*/
        }
    }
}

void MetalViewportRenderer::draw_image(
    viewport::RendererInterfacePtr &renderer_interface,
    const media_reader::ImageBufPtr &image_to_be_drawn,
    const media_reader::ImageSetLayoutDataPtr &layout_data,
    const int index,
    const Imath::M44f &window_to_viewport_matrix,
    const Imath::M44f &viewport_to_image_space,
    const float viewport_du_dx) {

    MetalRendererInterface *stateInfo = static_cast<MetalRendererInterface *>(renderer_interface.get());

    std::cerr << "active_shader_program_ " << active_shader_program_ << std::endl;

    id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>) stateInfo->command_encoder;
    
    active_shader_program_->bind_uniforms(encoder);

    std::cerr << "OINK " << active_shader_program_->vertexFunction() << std::endl;
    render_pipe_->rpDesc.vertexFunction = active_shader_program_->vertexFunction();
    render_pipe_->rpDesc.fragmentFunction = active_shader_program_->fragmentFunction();
    std::cerr << "A\n" << std::endl;
    NSError *err = nullptr;
    MTLRenderPipelineReflection *local_reflection = nullptr;
    render_pipe_->pipeline_ = [render_pipe_->device_ newRenderPipelineStateWithDescriptor: render_pipe_->rpDesc
                                                      options: MTLPipelineOptionArgumentInfo | MTLPipelineOptionBufferTypeInfo
                                                   reflection: &local_reflection
                                                        error: &err];
    std::cerr << "B\n" << std::endl;

    render_pipe_->reflection = local_reflection;
    if (!render_pipe_->pipeline_) {
        std::cerr << "Error creating pipeline: " << err.localizedDescription.UTF8String << std::endl;
        return;
        //NSAlert *anAlert = [NSAlert alertWithError:err];
        //[anAlert runModal];
    } else {
        active_shader_program_->load_uniform_layout(render_pipe_->reflection);
    }
    std::cerr << "C\n" << std::endl;

    init_shader_uniforms(
        image_to_be_drawn,
        window_to_viewport_matrix,
        viewport_to_image_space,
        viewport_du_dx,
        layout_data->custom_layout_data_,
        index);

    std::cerr << "@STAGE " << std::endl;

    //[encoder setFragmentBuffer: ubuf_[stateInfo->currentFrameSlot] offset: 0 atIndex: 0];
    [encoder setVertexBuffer: render_pipe_->vbuf_ offset: 0 atIndex: 1];
    [encoder setRenderPipelineState: render_pipe_->pipeline_];
    [encoder drawPrimitives: MTLPrimitiveTypeTriangleStrip vertexStart: 0 vertexCount: 4 instanceCount: 1 baseInstance: 0];

    /*active_shader_program_->use();

    // set-up core shader parameters (e.g. image transform matrix etc)

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

std::cerr << "C" << std::endl;
    if (!virt_image_buffer_unpack_shader ||
        virt_image_buffer_unpack_shader->graphics_api() != GraphicsAPI::Metal) {
        spdlog::warn("{} {}", __PRETTY_FUNCTION__, "No shader passed with image buffer.");
        return false;
    }
std::cerr << "C" << std::endl;

    auto image_buffer_unpack_shader =
        static_cast<metal::MetalShader const *>(virt_image_buffer_unpack_shader.get());
    if (!image_buffer_unpack_shader) {
    }
std::cerr << "C" << std::endl;

    std::string shader_id = to_string(image_buffer_unpack_shader->shader_id());

    for (const auto &op : colour_operations) {
        shader_id += op->cache_id();
    }
std::cerr << "C " << shader_id << std::endl;

    // do we already have this shader compiled?
    if (shader_programs_.find(shader_id) == shader_programs_.end()) {

        // try to compile the shader for this combo of image buffer unpack
        // and colour pipeline components

        try {

            std::vector<std::string> shader_components;
            for (const auto &colour_op : colour_operations) {
                // sanity check - this should be impossible, though
                if (colour_op->shader_ && colour_op->shader_->graphics_api() != GraphicsAPI::Metal) {
                    throw std::runtime_error(
                        "Non-Metal shader data in colour operation chain!");
                }
                std::cerr << "cp " << colour_op->cache_id() << " " << colour_op->shader_ << std::endl;
                auto pr = static_cast<metal::MetalShader const *>(colour_op->shader_.get());
                if (pr) {
                    std::cerr << "Adding shader component: " << pr->shader_code() << std::endl;
                    shader_components.push_back(pr->shader_code());
                }
            }

            /*shader_programs_[shader_id].reset(new MetalShaderProgram(
                default_vertex_shader,
                image_buffer_unpack_shader->shader_code(),
                shader_components,
                use_ssbo_));*/

        } catch (std::exception &e) {
            spdlog::error("{}", e.what());
            shader_programs_[shader_id].reset();
        }
    }

std::cerr << "D" << std::endl;
    if (shader_programs_[shader_id]) {
        active_shader_program_ = shader_programs_[shader_id];
    } else {
        active_shader_program_ = no_image_shader_program_;
    }
active_shader_program_ = no_image_shader_program_;

    std::cerr << this << " no_image_shader_program_ " << no_image_shader_program_ << std::endl;
std::cerr << "active_shader_program_ " << active_shader_program_ << std::endl;
    return active_shader_program_ != no_image_shader_program_;
}

void MetalViewportRenderer::init_shader_uniforms(
    const media_reader::ImageBufPtr &image_to_be_drawn,
    const Imath::M44f &window_to_viewport_matrix,
    const Imath::M44f &viewport_to_image_space,
    const float viewport_du_dx,
    const utility::JsonStore &layout_data,
    const int image_index) const {
    
    try {
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
    } 
}

void MetalViewportRenderer::do_init(RendererInterfacePtr &renderer_interface) { 

    static const float vertices[] = {
        -1, -1,
        1, -1,
        -1, 1,
        1, 1
    };

    MetalRendererInterface *stateInfo = static_cast<MetalRendererInterface *>(renderer_interface.get());

    assert(stateInfo->framesInFlight <= 3);
    render_pipe_->device_ = (__bridge id<MTLDevice>) stateInfo->device;
    render_pipe_->vbuf_ = [render_pipe_->device_ newBufferWithLength: sizeof(vertices) options: MTLResourceStorageModeShared];
    void *p = [render_pipe_->vbuf_ contents];
    memcpy(p, vertices, sizeof(vertices));

    /*for (int i = 0; i < stateInfo->framesInFlight; ++i)
        ubuf_[i] = [device_ newBufferWithLength: UBUF_SIZE options: MTLResourceStorageModeShared];*/

    MTLVertexDescriptor *inputLayout = [MTLVertexDescriptor vertexDescriptor];
    inputLayout.attributes[0].format = MTLVertexFormatFloat2;
    inputLayout.attributes[0].offset = 0;
    inputLayout.attributes[0].bufferIndex = 1; // ubuf is 0, vbuf is 1
    inputLayout.layouts[1].stride = 2 * sizeof(float); // vbuf layout
    //inputLayout.layouts[0].stride = UBUF_SIZE; // ubuf layout

    render_pipe_->rpDesc = [[MTLRenderPipelineDescriptor alloc] init];
    render_pipe_->rpDesc.vertexDescriptor = inputLayout;

    //active_shader_program_.reset(new MetalShaderProgram(device_, vertexShader, fragmentShader, true));

    //rpDesc.vertexFunction = active_shader_program_->vertexFunction();
    //rpDesc.fragmentFunction = active_shader_program_->fragmentFunction();

    render_pipe_->rpDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    render_pipe_->rpDesc.colorAttachments[0].blendingEnabled = true;
    render_pipe_->rpDesc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    render_pipe_->rpDesc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    render_pipe_->rpDesc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOne;
    render_pipe_->rpDesc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;

    if (render_pipe_->device_.depth24Stencil8PixelFormatSupported) {
        render_pipe_->rpDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth24Unorm_Stencil8;
        render_pipe_->rpDesc.stencilAttachmentPixelFormat = MTLPixelFormatDepth24Unorm_Stencil8;
    } else
    {
        render_pipe_->rpDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
        render_pipe_->rpDesc.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    }

    std::cerr << "A\n";

    std::cerr << "B\n";

    // add shader for no image render
    try {
        no_image_shader_program_ =
            MetalShaderProgramPtr(static_cast<MetalShaderProgram *>(new NoImageShaderProgram(render_pipe_->device_)));
    } catch (std::exception &e) {
        spdlog::critical("{} {}", __PRETTY_FUNCTION__, e.what());
    }

    std::cerr << this << "no_image_shader_program_ " << no_image_shader_program_ << std::endl;
    // resources_->init(); 
}

