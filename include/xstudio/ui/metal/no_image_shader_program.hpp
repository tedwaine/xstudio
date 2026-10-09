// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "xstudio/ui/metal/metal_shader_program.hpp"

namespace xstudio::ui::metal {

class NoImageShaderProgram : public MetalShaderProgram {
  public:
    NoImageShaderProgram(id<MTLDevice> device);
};
} // namespace xstudio::ui::metal
