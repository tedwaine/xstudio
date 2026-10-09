#include <Metal/Metal.h>
#include <AppKit/AppKit.h>

#include "xstudio/ui/metal/metal_shader_program.hpp"
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
const std::string baseVertexShader = R"(
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

struct uniform_data
{
    int2 image_dims;
    float4x4 image_transform_matrix;
    float4x4 to_coord_system;
    float4x4 to_canvas;
    float image_aspect;
    int2 image_bounds_min;
    int2 image_bounds_max;
};

vertex main0_out main0(main0_in in [[stage_in]], constant uniform_data& udata [[buffer(0)]])
{
    main0_out out = {};

    // awkward scale/translate to accommodate overscan where image_bounds (i.e.
    // exr data window) is different to image_dims (i.e. display window size)
    // This could/should go in image_transform_matrix!
    float bdbx = float(udata.image_bounds_max.x-udata.image_bounds_min.x);
    float bdby = float(udata.image_bounds_max.y-udata.image_bounds_min.y);
    float alpha = float(udata.image_bounds_min.x + udata.image_bounds_max.x)/float(udata.image_dims.x) - 1.0f;
    float beta = bdbx/float(udata.image_dims.x);
    float alpha_y = float(udata.image_bounds_min.y + udata.image_bounds_max.y)/float(udata.image_dims.y) - 1.0f;
    float beta_y = bdby/float(udata.image_dims.y);
    float4 rpos = in.vertices;

    rpos.x = alpha + beta*rpos.x;
    rpos.y = alpha_y + beta_y*rpos.y;
    rpos.y = rpos.y/udata.image_aspect;

    out.gl_Position = rpos*udata.image_transform_matrix*udata.to_coord_system*udata.to_canvas;
    out.coords = float2(udata.image_bounds_min.x + bdbx*(in.vertices.x + 1.0f)*0.5f, udata.image_bounds_min.y + bdby*(in.vertices.y + 1.0f)*0.5f);

    return out;
}
)";

const std::string baseFragmentShader = R"(
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
    out.fragColor = float4((in.coords * 0.5) + float2(0.5), i, i)*2.0;
    return out;
})";
} // anon namespace

id<MTLFunction> MetalShaderProgram::compileShaderFromSource(const std::string &src, const std::string &entryPoint)
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

MetalShaderProgram::MetalShaderProgram(
    id<MTLDevice> device,
    const std::string &vertex_shader,
    const std::string &fragment_shader,
    const bool do_compile)
    : device_(device), vertex_shaders_({vertex_shader}), fragment_shaders_({fragment_shader})
{
    if (do_compile) {
        compile();
    }
}

MetalShaderProgram::~MetalShaderProgram() {
    // Destructor implementation here
}

void MetalShaderProgram::compile() {
    // Method implementation here
    vertex_shader_ = compileShaderFromSource(vertex_shaders_.back(), "main0");
    fragment_shader_ = compileShaderFromSource(fragment_shaders_.back(), "main0");
}

void MetalShaderProgram::inject_colour_op_shader(const std::string &colour_op_shader) {
    // Method implementation here
}

bool MetalShaderProgram::set_type_info(MTLDataType type, UniformMember &m) {
    switch (type) {
    case MTLDataTypeFloat:    m.kind = 'f'; m.rows = 1; m.cols = 1; return true;
    case MTLDataTypeFloat2:   m.kind = 'f'; m.rows = 2; m.cols = 1; return true;
    case MTLDataTypeFloat3:   m.kind = 'f'; m.rows = 3; m.cols = 1; return true;
    case MTLDataTypeFloat4:   m.kind = 'f'; m.rows = 4; m.cols = 1; return true;
    case MTLDataTypeInt:      m.kind = 'i'; m.rows = 1; m.cols = 1; return true;
    case MTLDataTypeInt2:     m.kind = 'i'; m.rows = 2; m.cols = 1; return true;
    case MTLDataTypeInt3:     m.kind = 'i'; m.rows = 3; m.cols = 1; return true;
    case MTLDataTypeInt4:     m.kind = 'i'; m.rows = 4; m.cols = 1; return true;
    case MTLDataTypeUInt:     m.kind = 'u'; m.rows = 1; m.cols = 1; return true;
    case MTLDataTypeUInt2:    m.kind = 'u'; m.rows = 2; m.cols = 1; return true;
    case MTLDataTypeUInt3:    m.kind = 'u'; m.rows = 3; m.cols = 1; return true;
    case MTLDataTypeUInt4:    m.kind = 'u'; m.rows = 4; m.cols = 1; return true;
    case MTLDataTypeBool:     m.kind = 'b'; m.rows = 1; m.cols = 1; return true;
    case MTLDataTypeFloat2x2: m.kind = 'f'; m.rows = 2; m.cols = 2; return true;
    case MTLDataTypeFloat3x3: m.kind = 'f'; m.rows = 3; m.cols = 3; return true;
    case MTLDataTypeFloat4x4: m.kind = 'f'; m.rows = 4; m.cols = 4; return true;
    default: return false;
    }
}

void MetalShaderProgram::load_uniform_layout(MTLRenderPipelineReflection *reflection) {

    std::cerr << "load_uniform_layout " << std::endl;

    uniforms_.clear();
    uniform_blocks_.clear();

    auto add_stage = [&](NSArray<MTLArgument *> *args, bool vertex) {
        for (MTLArgument *arg in args) {
            if (arg.type != MTLArgumentTypeBuffer || arg.bufferDataType != MTLDataTypeStruct)
                continue;

            uniform_blocks_.push_back(
                {vertex, arg.index, std::vector<uint8_t>(arg.bufferDataSize, 0)});

            for (MTLStructMember *member in arg.bufferStructType.members) {
                UniformMember um;
                um.block  = uniform_blocks_.size() - 1;
                um.offset = member.offset;
                MTLDataType type = member.dataType;
                if (type == MTLDataTypeArray) {
                    um.array_length = member.arrayType.arrayLength;
                    um.stride       = member.arrayType.stride;
                    type            = member.arrayType.elementType;
                }
                if (set_type_info(type, um)) {
                    std::cerr << [member.name UTF8String] << " " << type << std::endl;
                    uniforms_[[member.name UTF8String]].push_back(um);
                }
            }
        }
    };

    add_stage(reflection.vertexArguments, true);
    add_stage(reflection.fragmentArguments, false);
}

void MetalShaderProgram::write_uniform(const UniformMember &m, const nlohmann::json &value) {

    auto &data = uniform_blocks_[m.block].data;

    const size_t comp_size  = m.kind == 'b' ? 1 : 4;
    const size_t col_stride = comp_size * (m.cols > 1 && m.rows == 3 ? 4 : m.rows);
    const size_t elem_bytes = m.cols > 1 ? col_stride * m.cols : comp_size * m.rows;
    const size_t elem_comps = m.rows * m.cols;

    size_t count = 1;
    size_t first = 0;
    if (value.is_array()) {
        if (value.size() < 2 || !value[1].is_number_integer())
            return;
        count = std::min<size_t>(value[1].get<size_t>(), std::max<size_t>(m.array_length, 1));
        first = 2;
        if (value.size() < first + count * elem_comps)
            return;
    } else if (elem_comps != 1 || !(value.is_number() || value.is_boolean())) {
        return;
    }

    if (count == 0 || m.offset + (count - 1) * m.stride + elem_bytes > data.size())
        return;

    for (size_t i = 0; i < count; ++i) {
        uint8_t *elem = data.data() + m.offset + i * m.stride;
        for (int c = 0; c < m.cols; ++c) {
            for (int r = 0; r < m.rows; ++r) {
                const int src = m.cols > 1 ? (transpose_matrices_ ? r * m.cols + c : c * m.rows + r) : r;
                const nlohmann::json &v =
                    value.is_array() ? value[first + i * elem_comps + src] : value;
                uint8_t *dst = elem + c * col_stride + r * comp_size;

                switch (m.kind) {
                case 'f': {
                    const float x = v.get<float>();
                    memcpy(dst, &x, sizeof(x));
                    break;
                }
                case 'i': {
                    const int32_t x = v.get<int32_t>();
                    memcpy(dst, &x, sizeof(x));
                    break;
                }
                case 'u': {
                    const uint32_t x = v.get<uint32_t>();
                    memcpy(dst, &x, sizeof(x));
                    break;
                }
                default:
                    *dst = (v.is_boolean() ? v.get<bool>() : v.get<int>() != 0) ? 1 : 0;
                }
            }
        }
    }
}

void MetalShaderProgram::set_shader_parameters(const utility::JsonStore &shader_params) {

    if (!shader_params.is_object())
        return;

    for (auto it = shader_params.begin(); it != shader_params.end(); ++it) {
        const auto found = uniforms_.find(it.key());
        if (found == uniforms_.end()) {
            std::cerr << "Uniform not found: " << it.key() << std::endl;
            continue;
        }
        std::cerr << "Setting uniform: " << it.key() << std::endl;
        for (const auto &member : found->second) {
            try {
                write_uniform(member, it.value());
            } catch (std::exception &e) {
                spdlog::warn("MetalShaderProgram: bad value for \"{}\": {}", it.key(), e.what());
            }
        }
    }
}

void MetalShaderProgram::bind_uniforms(id<MTLRenderCommandEncoder> encoder) {
    for (const auto &block : uniform_blocks_) {
        if (block.data.empty() || block.data.size() > 4096)
            continue;
        if (block.vertex) {
            [encoder setVertexBytes: block.data.data() length: block.data.size() atIndex: block.index];
        } else {
            std::cerr << "Binding fragment uniform block at index: " << block.index << " " <<block.data.size() << std::endl;
            [encoder setFragmentBytes: block.data.data() length: block.data.size() atIndex: block.index];
        }
    }
}

void MetalShaderProgram::set_shader_parameters(const media_reader::ImageBufPtr &image) {
    // Method implementation here
}