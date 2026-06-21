// SPDX-License-Identifier: Apache-2.0

#include "ffmpeg_pixel_unpack_shader.hpp"

using namespace xstudio;
using namespace xstudio::demo_plugin;

namespace {
static std::string shader_0{R"(
#version 410 core
uniform int width;
uniform int height;

// we need to forward declare this function, which is defined by the base
// gl shader class
vec2 get_image_data_2floats(int byte_address);

vec4 fetch_rgba_pixel(ivec2 image_coord)
{
    if (image_coord.x < 0 || image_coord.x >= width) return vec4(0.0,0.0,0.0,0.0);
    if (image_coord.y < 0 || image_coord.y >= height) return vec4(0.0,0.0,0.0,0.0);

    // 16 bytes per pixel (float RGBA)
    int pixel_address_bytes = (image_coord.x + image_coord.y*width)*16;

    vec2 pixRG = get_image_data_2floats(pixel_address_bytes);
    vec2 pixBA = get_image_data_2floats(pixel_address_bytes+8);
    
    return vec4(pixRG, pixBA);
}
)"};

// Credit for this shader is to user 'tubeman' on 'shadertoy.com'
// I have modified the code slightly so that the xSTUDIO 'reader'
// can set the colours and iMouse position based off some
// randomisation.
// https://www.shadertoy.com/user/tubeman
//
// Normally this shader would just be concerned with unpacking
// pixel data from the image buffer (that we generate here in)
// the media reader plugin) in RGBA values for a given image
// coordinate. xSTUDIO takes care of the rest (mainly colour
// transform and image display transform).
// This shader doesn't use the image buffer at all, however, and
// instead procedurally generates a colour at a given image
// position. See the other 'real' image readers in xSTUDIO for
// better examples of how to do this.
static std::string shader{R"(
#version 410 core
uniform int width;
uniform int height;
uniform float iTime;

uniform vec3 colour1;
uniform vec3 colour2;
uniform vec3 colour3;
uniform vec3 colour4;
uniform vec3 colour5;
uniform vec3 colour6;

uniform vec4 iMouse;

#define FAR 30.
#define PI 3.1415

int m = 0;

mat2 rot(float a) { float c = cos(a), s = sin(a); return mat2(c, -s, s, c); }
mat3 lookAt(vec3 dir) {
    vec3 up=vec3(0.,1.,0.);
    vec3 rt=normalize(cross(dir,up));
    return mat3(rt, cross(rt,dir), dir);
}

float gyroid(vec3 p) { return dot(cos(p), sin(p.zxy)) + 1.; }

float map(vec3 p) {
    float r = 1e5, d;
    
    d = gyroid(p);
    if (d<r) { r=d; m=1; }
    
    d = gyroid(p - vec3(0,0,PI));
    if (d<r) { r=d; m=2; }
    
    return r;
}

float raymarch(vec3 ro, vec3 rd) {
    float t = 0.;
    for (int i=0; i<150; i++) {
        float d = map(ro + rd*t);
        if (abs(d) < .001) break;
        t += d;
        if (t > FAR) break;
    }
    return t;
}

float getAO(vec3 p, vec3 sn){
	float occ = 0.;
    for (float i=0.; i<4.; i++) {
        float t = i*.08;        
        float d = map(p + sn*t);
        occ += t-d;
    }
    return clamp(1.-occ, 0., 1.);    
}

vec3 getNormal(vec3 p){
    vec2 e = vec2(0.5773,-0.5773)*0.001;
    return normalize(e.xyy*map(p+e.xyy) + e.yyx*map(p+e.yyx) + e.yxy*map(p+e.yxy) + e.xxx*map(p+e.xxx));
}

vec3 trace(vec3 ro, vec3 rd) {   
    vec3 C = vec3(0);
    vec3 throughput = vec3(1);
    
    for (int bounce = 0; bounce < 2; bounce++) {
        float d = raymarch(ro, rd);
        if (d > FAR) { break; }

        // fog
        float fog = 1. - exp(-.008*d*d);
        C += throughput * fog * vec3(0); throughput *= 1. - fog;        
        
        vec3 p = ro + rd*d;
        vec3 sn = normalize(getNormal(p) + pow(abs(cos(p*64.)), vec3(16))*.1);

        // lighting
        vec3 lp = vec3(10.,-10.,-10.+ro.z) ;
        vec3 ld = normalize(lp - p);
        float diff = max(0., .5+2.*dot(sn, ld));
        float diff2 = pow(length(sin(sn*2.)*.5+.5), 2.);
        float diff3 = max(0., .5+.5*dot(sn, vec2(1,0).yyx));
        
        float spec = max(0., dot(reflect(-ld, sn), -rd));
        float fres = 1. - max(0.,dot(-rd, sn));
        vec3 col = vec3(0), alb = vec3(0);
        
        col += colour1 * diff;
        col += colour2 * diff2;
        col += colour3 * diff3; 
        col += colour4 * pow(spec,4.)*8.;
        
        float freck = dot(cos(p*23.),vec3(1));
        if (m==1) { alb = colour5;  alb *= max(.6, step(2.5, freck)); }
        if (m==2) { alb = colour6;  alb *= max(.8, step(-2.5, freck)); }
        col *= alb;
        
        col *= getAO(p, sn);
        C += throughput * col;
            
        // reflection
        rd = reflect(rd, sn);
        ro = p + sn*.01;
        throughput *=  .9 * pow(fres, 1.);

    }
    return C;
}

vec4 fetch_rgba_pixel( ivec2 image_coord ) {

    vec2 iResolution = vec2(width, height);

    vec2 fragCoord = vec2(image_coord.x, image_coord.y);
    vec2 uv = (fragCoord.xy - iResolution.xy*.5) / iResolution.y;
    vec2 mo = (iMouse.xy - iResolution.xy*.5) / iResolution.y;

    vec3 ro = vec3(PI/2.,0, -iTime*.5);
    vec3 rd = normalize(vec3(uv, -.5));

    if (iMouse.z > 0.) {
        rd.zy = rot(mo.y*PI) * rd.zy;
        rd.xz = rot(-mo.x*PI) * rd.xz;
    } else {
        rd.xy = rot(sin(iTime*.2)) * rd.xy;
        vec3 ta = vec3(cos(iTime*.4), sin(iTime*.4), 4.);
        rd = lookAt(normalize(ta)) * rd;
    }
    
    vec3 col = trace(ro, rd);
    
    col *= smoothstep(0.,1., 1.2-length(uv*.9));
    col = pow(col, vec3(0.4545));
    return vec4(col, 1.0);
}

)"};

}

DemoPixelUnpackShader::DemoPixelUnpackShader(
    const utility::Uuid &shader_uuid
) : ui::opengl::OpenGLShader(shader_uuid, shader) {}