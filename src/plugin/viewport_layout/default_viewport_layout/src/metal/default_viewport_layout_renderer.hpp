// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "xstudio/ui/metal/metal_viewport_renderer.hpp"

namespace xstudio::ui::viewport {

/*
    The default viewport layout doesn't have any special rendering requirements
    so we can use the base OpenGLViewportRenderer.
*/
class DefaultViewportRenderer : public metal::MetalViewportRenderer {

  public:

    DefaultViewportRenderer(
        const std::string &window_id,
        const utility::JsonStore &prefs
    ) : MetalViewportRenderer(window_id, prefs) {}

    ~DefaultViewportRenderer() override = default;

};

} // namespace xstudio::ui::viewport
