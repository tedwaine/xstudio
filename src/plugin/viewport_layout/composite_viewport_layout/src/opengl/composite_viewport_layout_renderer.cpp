#include "xstudio/ui/opengl/opengl_viewport_renderer.hpp"
#include "xstudio/ui/opengl/opengl_multi_buffered_texture.hpp"
#include "composite_viewport_layout_renderer.hpp"

using namespace xstudio;
using namespace xstudio::ui::viewport;
using namespace xstudio::ui::opengl;

namespace {
const char *vertex_shader = R"(
    #version 330 core
    layout (location = 0) in vec4 aPos;
    uniform mat4 to_canvas;
    uniform mat4 canvas_to_image;
    out vec2 canvasCoordinate;
    out vec2 imageCoordinate;

    void main()
    {
        vec4 rpos = aPos;
        gl_Position = (rpos*to_canvas);
        vec4 ipos = (aPos*canvas_to_image);
        canvasCoordinate = (rpos.xy + vec2(1.0, 1.0))*0.5f;
        imageCoordinate = vec2(ipos.x/ipos.w, ipos.y/ipos.w);
    }
    )";

const char *frag_shader = R"(
    #version 410 core
    out vec4 FragColor;
    uniform sampler2D textureSamplerA;
    uniform sampler2D textureSamplerB;
    in vec2 canvasCoordinate;
    in vec2 imageCoordinate;
    uniform float boost;
    uniform bool screen;
    uniform bool monochrome;
    uniform float image_aspect;

    void main(void)
    {
        if (!screen && (imageCoordinate.x < -1.0 || imageCoordinate.x > 1.0 || imageCoordinate.y < -image_aspect || imageCoordinate.y > image_aspect)) {
            FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        } else {
            vec4 a = texture(textureSamplerA, canvasCoordinate);
            vec4 b = texture(textureSamplerB, canvasCoordinate);
            if (screen) {

                vec4 m = max(a, b);
                vec4 screen = vec4(1.0) - (vec4(1.0) - a)*(vec4(1.0) - b);
                FragColor = vec4(
                    m.x > 1.0 ? m.x : screen.x,
                    m.y > 1.0 ? m.y : screen.y,
                    m.z > 1.0 ? m.z : screen.z,
                    m.w > 1.0 ? m.w : screen.w
                    );

            } else if (monochrome) {
                float al = length(a.rgb);
                float bl = length(b.xyz);
                float scale = pow(2.0, boost);
                float d = (al - bl)*scale;
                FragColor = vec4(vec3(d) + vec3(0.5), 1.0);
            } else {
                float scale = pow(2.0, boost);
                vec4 d = (a - b)*scale;
                FragColor = vec4(d.xyz + vec3(0.5), 1.0);
            }
        }
    }

    )";
} // namespace

ViewportCompositeRenderer::ViewportCompositeRenderer(
    const std::string &window_id, const utility::JsonStore &prefs)
    : OpenGLViewportRenderer(window_id, prefs) {}

void ViewportCompositeRenderer::pre_init() {
    OpenGLViewportRenderer::pre_init();
    offscreen_texture_target_A_ = std::make_unique<OpenGLOffscreenRenderer>(GL_RGBA32F);
    offscreen_texture_target_B_ = std::make_unique<OpenGLOffscreenRenderer>(GL_RGBA32F);
    shader_ = std::make_unique<ui::opengl::GLShaderProgram>(vertex_shader, frag_shader);
}

void ViewportCompositeRenderer::draw_image(
    const media_reader::ImageBufPtr &image_to_be_drawn,
    const media_reader::ImageSetLayoutDataPtr &layout_data,
    const int index,
    const Imath::M44f &window_to_viewport_matrix,
    const Imath::M44f &viewport_to_image_space,
    const float viewport_du_dx) {

    active_shader_program_->use();

    int mode         = layout_data->custom_layout_data_.value("mode", 0);
    int first_im_idx = layout_data->custom_layout_data_.value("first_im", 0);

    if (mode == 3 || mode == 4) {

        render_difference(
            image_to_be_drawn,
            index == first_im_idx,
            window_to_viewport_matrix,
            viewport_to_image_space,
            viewport_du_dx,
            layout_data->custom_layout_data_);
        return;
    }
    // set-up core shader parameters (e.g. image transform matrix etc)
    init_shader_uniforms(
        image_to_be_drawn,
        window_to_viewport_matrix,
        viewport_to_image_space,
        viewport_du_dx,
        layout_data->custom_layout_data_,
        index);

    if (mode == 0) {
        float blend_ratio = layout_data->custom_layout_data_.value("blend_ratio", 0.5f);
        if (index == first_im_idx) {
            // we don't blend the first image, just draw it
            glDisable(GL_BLEND);
        } else {
            glBlendColor(blend_ratio, blend_ratio, blend_ratio, blend_ratio);
            glEnable(GL_BLEND);
            glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR);
            glBlendEquation(GL_FUNC_ADD);
        }
    } else {

        if (index == first_im_idx) {
            // we don't pre-mult the first image, just draw it
            glDisable(GL_BLEND);
        } else {
            static const utility::JsonStore j(nlohmann::json::parse(R"({"use_alpha": true})"));
            active_shader_program_->set_shader_parameters(j);
            glEnable(GL_BLEND);
            if (mode == 1) {
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
            } else {
                // 'Add' mode
                glBlendFunc(GL_ONE, GL_ONE);
            }
            glBlendEquation(GL_FUNC_ADD);
        }
    }

    // the actual draw .. a quad that spans -1.0, 1.0 in x & y.
    glBindVertexArray(vao());
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);

    glUseProgram(0);

    glDisable(GL_BLEND);
}

void ViewportCompositeRenderer::render_difference(
    const media_reader::ImageBufPtr &image_to_be_drawn,
    const bool first_im,
    const Imath::M44f &window_to_viewport_matrix,
    const Imath::M44f &viewport_to_image_space,
    const float viewport_du_dx,
    const utility::JsonStore &mode_params) {
    {
        auto offscreen_texture_target =
            first_im ? offscreen_texture_target_A_.get() : offscreen_texture_target_B_.get();

        // STEP 1 - render the viewport (for given image) to a texture
        offscreen_texture_target->resize(
            Imath::V2f(viewport_coords_in_window()[2], viewport_coords_in_window()[3]));
        offscreen_texture_target->begin();

        glDisable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        utility::JsonStore j;
        init_shader_uniforms(
            image_to_be_drawn, Imath::M44f(), viewport_to_image_space, viewport_du_dx, j, 0);

        // the actual draw .. a quad that spans -1.0, 1.0 in x & y.
        glBindVertexArray(vao());
        glEnableVertexAttribArray(0);
        glDisable(GL_BLEND);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(0);
        glBindVertexArray(0);

        glUseProgram(0);
        offscreen_texture_target->end();
    }

    // if this is first image, return as we only continue to render the difference
    // if we've got the 2nd image rendered to a texture
    if (first_im)
        return;


    // STEP 2 - render the difference image
    utility::JsonStore params;
    params["to_canvas"]       = window_to_viewport_matrix;
    params["canvas_to_image"] = viewport_to_image_space;
    params["textureSamplerA"] = 10;
    params["textureSamplerB"] = 11;
    params["monochrome"]      = mode_params.value("monochrome", true);
    params["boost"]           = mode_params.value("boost", 0.0f);
    params["screen"]          = mode_params.value("screen", false);
    params["image_aspect"]    = mode_params.value("image_aspect", 9.0f/16.0f);

    // set the active tex IDs for our texture targets
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, offscreen_texture_target_A_->texture_handle());
    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_2D, offscreen_texture_target_B_->texture_handle());

    shader_->use();
    shader_->set_shader_parameters(params);

    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);

    glBindVertexArray(vao());
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);

    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
