// SPDX-License-Identifier: Apache-2.0

#include <Metal/Metal.h>
#include <AppKit/AppKit.h>

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

namespace {
const std::string vertexShader = R"(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float2 coords [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 vertices [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = in.vertices;
    out.coords = in.vertices.xy;
    return out;
}
)";

const std::string fragmentShader = R"(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct buf
{
    float t;
};

struct main0_out
{
    float4 fragColor [[color(0)]];
};

struct main0_in
{
    float2 coords [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant buf& ubuf [[buffer(0)]])
{
    main0_out out = {};
    float i = 1.0 - (pow(abs(in.coords.x), 4.0) + pow(abs(in.coords.y), 4.0));
    i = smoothstep(ubuf.t - 0.800000011920928955078125, ubuf.t + 0.800000011920928955078125, i);
    i = floor(i * 20.0) / 20.0;
    out.fragColor = float4((in.coords * 0.5) + float2(0.5), i, i);
    return out;
})";
} // anon namespace

namespace xstudio::ui::metal {
class TestRenderer
{
    public:
    TestRenderer() = default;
    ~TestRenderer() = default;

    id<MTLFunction> compileShaderFromSource(const std::string &src, const std::string &entryPoint);
    void init(int framesInFlight, MetalRendererInterface *stateInfo);
    void render(MetalRendererInterface *stateInfo, const Imath::V2i &window_size);

    bool initialized_ = false;
    id<MTLDevice> device_;
    id<MTLBuffer> vbuf_;
    id<MTLBuffer> ubuf_[3];
    id<MTLFunction> vertex_shader_;
    id<MTLFunction> fragment_shader_;
    id<MTLRenderPipelineState> pipeline_;
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
    viewport::RendererInterfacePtr &renderer_interface,
    const media_reader::ImageBufDisplaySetPtr &images,
    const Imath::M44f &window_to_viewport_matrix,
    const Imath::M44f &viewport_to_image_space,
    const Imath::V2i &window_size,
    const float device_pixel_ratio,
    const std::vector<plugin::ViewportOverlayRendererPtr> &overlay_renderers) {

std::cerr << "Render " << window_id_ << " " << window_size.x << " " << window_size.y << " " << renderer_interface << "\n";
    if (!renderer_interface) {
        return;
    }

    MetalRendererInterface *stateInfo = static_cast<MetalRendererInterface *>(renderer_interface.get());
    if (!renderer_) {
        renderer_ = new TestRenderer();
    }
    renderer_->render(stateInfo, window_size);


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

void TestRenderer::render(MetalRendererInterface *stateInfo, const Imath::V2i &window_size)
{
    if (!initialized_) {
        init(stateInfo->framesInFlight, stateInfo);
    }

    id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>) stateInfo->command_encoder;
    
    std::cerr << "Render " << encoder << " " << stateInfo->currentFrameSlot << " " << window_size.x << " " << window_size.y << "\n";

    MTLViewport vp;
    vp.originX = 0;
    vp.originY = 0;
    vp.width = window_size.x;
    vp.height = window_size.y;
    vp.znear = 0;
    vp.zfar = 1;
    [encoder setViewport: vp];

    [encoder setFragmentBuffer: ubuf_[stateInfo->currentFrameSlot] offset: 0 atIndex: 0];
    [encoder setVertexBuffer: vbuf_ offset: 0 atIndex: 1];
    [encoder setRenderPipelineState: pipeline_];
    [encoder drawPrimitives: MTLPrimitiveTypeTriangleStrip vertexStart: 0 vertexCount: 4 instanceCount: 1 baseInstance: 0];

}

id<MTLFunction> TestRenderer::compileShaderFromSource(const std::string &src, const std::string &entryPoint)
{

    NSString *srcstr = [NSString stringWithCString: src.c_str() encoding:[NSString defaultCStringEncoding]];
    MTLCompileOptions *opts = [[MTLCompileOptions alloc] init];
    opts.languageVersion = MTLLanguageVersion1_2;
    NSError *err = nullptr;
    id<MTLLibrary> lib = [device_ newLibraryWithSource: srcstr options: opts error: &err];
    // srcstr is autoreleased, opts is managed by ARC

    if (err) {
        NSAlert *anAlert = [NSAlert alertWithError:err];
        [anAlert runModal];
        return nullptr;
    }

    NSString *name = [NSString stringWithCString: entryPoint.c_str() encoding:[NSString defaultCStringEncoding]];
    id<MTLFunction> fn = [lib newFunctionWithName: name];
    // [name release]; // NSString created with stringWithCString is autoreleased

    return fn;
}

static const float vertices[] = {
    -1, -1,
    1, -1,
    -1, 1,
    1, 1
};

const int UBUF_SIZE = 4;

void TestRenderer::init(int framesInFlight, MetalRendererInterface *stateInfo)
{

    assert(framesInFlight <= 3);
    initialized_ = true;
    std::cerr << "A " << framesInFlight << "\n";
    device_ = (__bridge id<MTLDevice>) stateInfo->device;
    vbuf_ = [device_ newBufferWithLength: sizeof(vertices) options: MTLResourceStorageModeShared];
    void *p = [vbuf_ contents];
    memcpy(p, vertices, sizeof(vertices));

    for (int i = 0; i < framesInFlight; ++i)
        ubuf_[i] = [device_ newBufferWithLength: UBUF_SIZE options: MTLResourceStorageModeShared];

    MTLVertexDescriptor *inputLayout = [MTLVertexDescriptor vertexDescriptor];
    inputLayout.attributes[0].format = MTLVertexFormatFloat2;
    inputLayout.attributes[0].offset = 0;
    inputLayout.attributes[0].bufferIndex = 1; // ubuf is 0, vbuf is 1
    inputLayout.layouts[1].stride = 2 * sizeof(float);

    MTLRenderPipelineDescriptor *rpDesc = [[MTLRenderPipelineDescriptor alloc] init];
    rpDesc.vertexDescriptor = inputLayout;

    rpDesc.vertexFunction = compileShaderFromSource(vertexShader, "main0");
    rpDesc.fragmentFunction = compileShaderFromSource(fragmentShader, "main0");

    rpDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    rpDesc.colorAttachments[0].blendingEnabled = true;
    rpDesc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    rpDesc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    rpDesc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOne;
    rpDesc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;

    if (device_.depth24Stencil8PixelFormatSupported) {
        rpDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth24Unorm_Stencil8;
        rpDesc.stencilAttachmentPixelFormat = MTLPixelFormatDepth24Unorm_Stencil8;
    } else
    {
        rpDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
        rpDesc.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    }

    NSError *err = nullptr;
    pipeline_ = [device_ newRenderPipelineStateWithDescriptor: rpDesc error: &err];
    if (!pipeline_) {
        NSAlert *anAlert = [NSAlert alertWithError:err];
        [anAlert runModal];
    }
}
