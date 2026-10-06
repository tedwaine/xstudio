// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <Metal/Metal.h>

#include <cstdint>
#include <map>
#include <vector>

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

    // Sets uniforms by name. Values are a JSON scalar, or [type, count, v0, v1, ...] for
    // vectors, matrices and arrays. Requires load_uniform_layout() to have been called.
    void set_shader_parameters(const utility::JsonStore &shader_params);
    void set_shader_parameters(const media_reader::ImageBufPtr &image);

    // Reflection must come from a pipeline created with MTLPipelineOptionArgumentInfo |
    // MTLPipelineOptionBufferTypeInfo.
    void load_uniform_layout(MTLRenderPipelineReflection *reflection);

    // Uploads the uniform blocks (each must be <= 4KB) to the encoder.
    void bind_uniforms(id<MTLRenderCommandEncoder> encoder);

    void set_transpose_matrices(bool transpose) { transpose_matrices_ = transpose; }

    id<MTLFunction> fragmentFunction() { return fragment_shader_; }
    id<MTLFunction> vertexFunction() { return vertex_shader_; }

    void compile();

  private:

    // A struct-typed buffer argument of the vertex or fragment function.
    struct UniformBlock {
        bool vertex;
        NSUInteger index;
        std::vector<uint8_t> data;
    };

    // A named member of a UniformBlock. kind is one of 'f', 'i', 'u', 'b'.
    struct UniformMember {
        size_t block        = 0;
        size_t offset       = 0;
        size_t array_length = 0; // 0 if not an array
        size_t stride       = 0; // array element stride in bytes
        char kind           = 'f';
        int rows            = 1;
        int cols            = 1; // > 1 for matrices
    };

    static bool set_type_info(MTLDataType type, UniformMember &member);
    void write_uniform(const UniformMember &member, const nlohmann::json &value);

    std::vector<UniformBlock> uniform_blocks_;
    std::map<std::string, std::vector<UniformMember>> uniforms_;

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
