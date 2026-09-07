// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <Metal/Metal.h>

// clang-format off
#include <Imath/ImathVec.h>
#include <Imath/ImathMatrix.h>
// clang-format on

#include "xstudio/utility/json_store.hpp"
#include "xstudio/media_reader/image_buffer.hpp"
#include "xstudio/ui/viewport/shader.hpp"

namespace xstudio::ui::metal {

class MetalShaderProgram {

  public:

    MetalShaderProgram(
        id<MTLDevice> device,
        const std::string &vertex_shader,
        const std::string &fragment_shader,
        const bool do_compile = true);

    ~MetalShaderProgram();

    void inject_colour_op_shader(const std::string &colour_op_shader);

    void set_shader_parameters(const utility::JsonStore &shader_params);
    void set_shader_parameters(const media_reader::ImageBufPtr &image);

    void set_transpose_matrices(bool transpose) { transpose_matrices_ = transpose; }

    id<MTLFunction> fragmentFunction() { return fragment_shader_; }
    id<MTLFunction> vertexFunction() { return vertex_shader_; }

    void compile();

  private:

    id<MTLFunction> compileShaderFromSource(const std::string &src, const std::string &entryPoint);

    int get_param_location(const std::string &param_name);
    std::map<std::string, int> locations_;
    std::vector<std::string> vertex_shaders_;
    std::vector<std::string> fragment_shaders_;
    std::vector<std::string> orig_fragment_shaders_;
    id<MTLFunction> vertex_shader_;
    id<MTLFunction> fragment_shader_;
    id<MTLDevice> device_;
    int colour_operation_index_ = {1};
    bool transpose_matrices_    = {false};
};

typedef std::shared_ptr<MetalShaderProgram> MetalShaderProgramPtr;

} // namespace xstudio::ui::metal
