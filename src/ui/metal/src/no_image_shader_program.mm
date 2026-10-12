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

struct unpack_uniforms
{
    int2 texture_dims;
    int frame_width_pixels;
    int pix_fmt;
    int y_linesize;
    int u_linesize;
    int v_linesize;
    int a_linesize;
    int y_plane_bytes_offset;
    int u_plane_bytes_offset;
    int v_plane_bytes_offset;
    int a_plane_bytes_offset;
    bool half_scale_uvy;
    bool half_scale_uvx;
    int bits_per_channel;
    float3x3 yuv_conv;
    float3 yuv_offsets;
    float norm_coeff;

};

extern "C" float4 computeCustomEffects(int2 texCoords, constant unpack_uniforms& tt, const device uint8_t *foo);

fragment main0_out main0(
    main0_in in [[stage_in]],
    constant buf& ubuf [[buffer(0)]],
    const device uint8_t *foo [[buffer(1)]],
    constant unpack_uniforms& unif [[buffer(2)]]
){
    main0_out out = {};
    int2 texCoords = int2(round(in.coords.x), round(in.coords.y));
    out.fragColor = computeCustomEffects(texCoords, unif, foo);
    return out;
})";
} // anon namespace

NoImageShaderProgram::NoImageShaderProgram(id<MTLDevice> device)
    : MetalShaderProgram(device, baseVertexShader, baseFragmentShader, true) 
{

}
