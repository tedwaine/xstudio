#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

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

extern "C" float4 computeCustomEffects(int2 texCoords, constant unpack_uniforms& tt, const device uint8_t *foo) {

	int2 uv_coord(
		tt.half_scale_uvx ? texCoords.x >> 1 : texCoords.x,
		tt.half_scale_uvy ? texCoords.y >> 1 : texCoords.y
		);

    uint3 yuv(
        foo[tt.y_plane_bytes_offset + texCoords.x + (texCoords.y * tt.y_linesize)],
        foo[tt.u_plane_bytes_offset + uv_coord.x + (uv_coord.y * tt.u_linesize)],
        foo[tt.v_plane_bytes_offset + uv_coord.x + (uv_coord.y * tt.v_linesize)]
    );

    float3 rgb = ((float3(yuv) - tt.yuv_offsets) * tt.yuv_conv) * tt.norm_coeff;
    return float4(rgb, 1.0);


}