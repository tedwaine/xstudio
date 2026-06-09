// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "xstudio/ui/opengl/opengl_viewport_renderer.hpp"

namespace xstudio::ui::viewport {

/*
    The grid viewport layout doesn't have any special rendering requirements
    so we can use the base OpenGLViewportRenderer for it, which is capable
    of rendering muultiple images with a transform but no other special
    compositing.
*/
class ViewportGridRenderer : public opengl::OpenGLViewportRenderer {

  public:

    ViewportGridRenderer(
        const std::string &window_id,
        const utility::JsonStore &prefs
    ) : OpenGLViewportRenderer(window_id, prefs) {}

    ~ViewportGridRenderer() override = default;

};

} // namespace xstudio::ui::viewport
