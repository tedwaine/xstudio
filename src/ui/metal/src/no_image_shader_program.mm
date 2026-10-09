// SPDX-License-Identifier: Apache-2.0
#include <sstream>

#include "xstudio/ui/metal/no_image_shader_program.hpp"

using namespace xstudio::ui::metal;

namespace {
const std::string baseVertexShader = R"(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
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

NoImageShaderProgram::NoImageShaderProgram()
    : MetalShaderProgram(baseVertexShader, baseFragmentShader, true) 
{

}
