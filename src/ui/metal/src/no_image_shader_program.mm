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
    float to_display_exposure_contrast_exposureVal;

};

vertex main0_out main0(main0_in in [[stage_in]], constant uniform_data& udata [[buffer(0)]])
{
    main0_out out = {};

    // awkward scale/translate to accommodate overscan where image_bounds (i.e.
    // exr data window) is  different to image_dims (i.e. display window size)
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

    out.coords = float2(rpos.x, rpos.y);

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
    float4x4 image_transform_matrix;
    float to_display_exposure_contrast_exposureVal;
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
    out.fragColor = float4(in.coords.x, in.coords.y, 0.5+ubuf.to_display_exposure_contrast_exposureVal, 1.0);
    return out;
})";
} // anon namespace

NoImageShaderProgram::NoImageShaderProgram(id<MTLDevice> device)
    : MetalShaderProgram(device, baseVertexShader, baseFragmentShader, true) 
{

}
