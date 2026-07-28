// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <memory>
#include <OpenColorIO/OpenColorIO.h> //NOLINT

#include "xstudio/plugin_manager/plugin_base.hpp"

namespace OCIO = OCIO_NAMESPACE;

namespace xstudio::ui::viewport {

    /*
    The pre_viewport_draw_gpu_hook is called with the GL context of the
    viewport in an active state. We draw the strokes of the grading mask
    into a GL texture, and set the texture ID on the colour pipeline data
    of the image that is passed in. When the image is drawn to the screen
    our shader can sample the texture to mask the grade.
    */

    class GradingMaskRenderer : public plugin::GPUPreDrawHook {

      public:

        GradingMaskRenderer(const std::string viewport_name) {}

        void pre_viewport_draw_gpu_hook(
            const Imath::M44f &transform_window_to_viewport_space,
            const Imath::M44f &transform_viewport_to_image_space,
            const float viewport_du_dpixel,
            xstudio::media_reader::ImageBufPtr &image) override {}

        static GPUShaderPtr make_mask_shader(
            OCIO::ConstGpuShaderDescRcPtr &lin_to_log_shader_desc,
            OCIO::ConstGpuShaderDescRcPtr &log_to_lin_shader_desc,
            size_t &hash
        ) { return nullptr; }

      private:

    };

    using GradingMaskRendererSPtr = std::shared_ptr<GradingMaskRenderer>;

} // end namespace viewport
