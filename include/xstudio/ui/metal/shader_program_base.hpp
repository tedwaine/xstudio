// SPDX-License-Identifier: Apache-2.0
#pragma once

// clang-format off
#include <Imath/ImathVec.h>
#include <Imath/ImathMatrix.h>
// clang-format on

#include "xstudio/utility/json_store.hpp"
#include "xstudio/media_reader/image_buffer.hpp"
#include "xstudio/ui/viewport/shader.hpp"

namespace xstudio::ui::metal {

class MetalShader : public viewport::GPUShader {
  public:
    MetalShader(utility::Uuid id, std::string code)
        : viewport::GPUShader(id, viewport::GraphicsAPI::Metal),
          shader_code_(std::move(code)) {}

    [[nodiscard]] const std::string &shader_code() const { return shader_code_; }

  private:
    const std::string shader_code_;
};

} // namespace xstudio::ui::metal
