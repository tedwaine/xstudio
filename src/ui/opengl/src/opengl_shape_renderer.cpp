// SPDX-License-Identifier: Apache-2.0

#include "xstudio/ui/opengl/opengl_shape_renderer.hpp"

using namespace xstudio::ui::canvas;
using namespace xstudio::ui::opengl;


namespace {

const char *vertex_shader = R"(
    #version 430 core

    in vec4 vertices;
    out vec2 coords;

    const vec2 positions[4] = {
        vec2(-1, -1),
        vec2( 1, -1),
        vec2(-1,  1),
        vec2( 1,  1),
    };

    void main() {
        gl_Position = vec4(positions[gl_VertexID].xy, 0.0, 1.0);
        coords = vec2(gl_Position.x, -gl_Position.y);
    }
)";

const char *frag_shader = R"(
    #version 430

    // TODO: Turn these shapes uniform arrays into proper buffer
    // to have more flexibility in terms of maximum number available.

    struct Quad {
        vec2 tl;
        vec2 tr;
        vec2 br;
        vec2 bl;
        vec3 color;
        float opacity;
        float softness;
        float invert;
    };

    struct Ellipse {
        vec2 center;
        vec2 radius;
        float angle;
        vec3 color;
        float opacity;
        float softness;
        float invert;
    };

    const int MAX_POINTS_PER_POLYGON = 32;
    struct Polygon {
        vec2 points[MAX_POINTS_PER_POLYGON];
        int count;
        vec3 color;
        float opacity;
        float softness;
        float invert;
    };

    const int MAX_QUADS = 16;
    uniform int quads_count;
    uniform Quad quads[MAX_QUADS];

    const int MAX_ELLIPSES = 16;
    uniform int ellipses_count;
    uniform Ellipse ellipses[MAX_ELLIPSES];

    const int MAX_POLYGONS = 16;
    uniform int polygons_count;
    uniform Polygon polygons[MAX_POLYGONS];

    uniform float image_aspectratio;

    in vec2 coords;
    out vec4 color;

    /////////////////////////////////////////
    // License for sdQuad() sdPolygon()
    /////////////////////////////////////////

    // The MIT License
    // Copyright © 2021 Inigo Quilez
    // Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    // https://www.shadertoy.com/view/7dSGWK
    float sdQuad(vec2 p, vec2 p0, vec2 p1, vec2 p2, vec2 p3)
    {
        vec2 e0 = p1 - p0; vec2 v0 = p - p0;
        vec2 e1 = p2 - p1; vec2 v1 = p - p1;
        vec2 e2 = p3 - p2; vec2 v2 = p - p2;
        vec2 e3 = p0 - p3; vec2 v3 = p - p3;

        vec2 pq0 = v0 - e0*clamp( dot(v0,e0)/dot(e0,e0), 0.0, 1.0 );
        vec2 pq1 = v1 - e1*clamp( dot(v1,e1)/dot(e1,e1), 0.0, 1.0 );
        vec2 pq2 = v2 - e2*clamp( dot(v2,e2)/dot(e2,e2), 0.0, 1.0 );
        vec2 pq3 = v3 - e3*clamp( dot(v3,e3)/dot(e3,e3), 0.0, 1.0 );
        
        vec2 ds = min( min( vec2( dot( pq0, pq0 ), v0.x*e0.y-v0.y*e0.x ),
                            vec2( dot( pq1, pq1 ), v1.x*e1.y-v1.y*e1.x )),
                    min( vec2( dot( pq2, pq2 ), v2.x*e2.y-v2.y*e2.x ),
                            vec2( dot( pq3, pq3 ), v3.x*e3.y-v3.y*e3.x ) ));

        float d = sqrt(ds.x);

        return (ds.y>0.0) ? -d : d;
    }

    // https://iquilezles.org/articles/distfunctions2d/
    float sdPolygon(vec2 p, vec2[MAX_POINTS_PER_POLYGON] v, uint count)
    {
        float d = dot(p-v[0],p-v[0]);
        float s = 1.0;
        for( uint i=0, j=count-1; i<count; j=i, i++ )
        {
            vec2 e = v[j] - v[i];
            vec2 w =    p - v[i];
            vec2 b = w - e*clamp( dot(w,e)/dot(e,e), 0.0, 1.0 );
            d = min( d, dot(b,b) );
            bvec3 c = bvec3(p.y>=v[i].y,p.y<v[j].y,e.x*w.y>e.y*w.x);
            if( all(c) || all(not(c)) ) s*=-1.0;
        }
        return s*sqrt(d);
    }

    // https://blog.chatfield.io/simple-method-for-distance-to-ellipse/ trig-less version
    // https://github.com/0xfaded/ellipse_demo/issues/1
    // https://www.shadertoy.com/view/tt3yz7
    float sdEllipse4(vec2 p, vec2 e)
    {
        vec2 pAbs = abs(p);
        vec2 ei = 1.0 / e;
        vec2 e2 = e*e;
        vec2 ve = ei * vec2(e2.x - e2.y, e2.y - e2.x);

        // cos/sin(math.pi / 4)
        vec2 t = vec2(0.70710678118654752, 0.70710678118654752);

        for (int i = 0; i < 3; i++) {
            vec2 v = ve*t*t*t;
            vec2 u = normalize(pAbs - v) * length(t * e - v);
            vec2 w = ei * (v + u);
            t = normalize(clamp(w, 0.0, 1.0));
        }

        vec2 nearestAbs = t * e;
        float dist = length(pAbs - nearestAbs);
        return dot(pAbs, pAbs) < dot(nearestAbs, nearestAbs) ? -dist : dist;
    }

    float opRound(float shape, float r)
    {
        return shape - r;
    }

    vec2 opRotate(vec2 coord, float degrees)
    {
        float rad = radians(degrees);

        mat2 scale_fwd = mat2(
            1.0f, 0.0,
            0.0, image_aspectratio
        );
        mat2 scale_inv = mat2(
            1.0f, 0.0,
            0.0, 1.0f / image_aspectratio
        );
        mat2 rotation_mat = mat2(
            cos(rad),-sin(rad),
            sin(rad), cos(rad)
        );
        return coord * scale_inv * rotation_mat * scale_fwd;
    }

    void main()
    {
        vec4 accum_color = vec4(0.0f);

        // Interactive shapes from QML

        for (int i = 0; i < quads_count; ++i) {
            Quad q = quads[i];
            float softness = q.softness;
            float d = sdQuad(coords, q.bl, q.tl, q.tr, q.br) * q.invert;
            float d_smooth = opRound(d, softness);
            vec4 color = mix(vec4(0.0f), vec4(q.color, 1.0f), q.opacity);
            accum_color = max(accum_color, mix(color, vec4(0.0f), smoothstep(0.0f, d - d_smooth, d)));
        }

        for (int i = 0; i < ellipses_count; ++i) {
            Ellipse e = ellipses[i];
            float softness = e.softness;
            float d = sdEllipse4(opRotate(coords - e.center, e.angle), e.radius) * e.invert;
            float d_smooth = opRound(d, softness);
            vec4 color = mix(vec4(0.0f), vec4(e.color, 1.0f), e.opacity);
            accum_color = max(accum_color, mix(color, vec4(0.0f), smoothstep(0.0f, d - d_smooth, d)));
        }

        for (int i = 0; i < polygons_count; ++i) {
            Polygon p = polygons[i];
            float softness = p.softness;
            float d = sdPolygon(coords, p.points, p.count) * p.invert;
            float d_smooth = opRound(d, softness);
            vec4 color = mix(vec4(0.0f), vec4(p.color, 1.0f), p.opacity);
            accum_color = max(accum_color, mix(color, vec4(0.0f), smoothstep(0.0f, d - d_smooth, d)));
        }

        color = accum_color;
    }
)";

} // anonymous namespace


OpenGLShapeRenderer::~OpenGLShapeRenderer() { cleanup_gl(); }

void OpenGLShapeRenderer::init_gl() {

    if (!shader_) {
        shader_ = std::make_unique<ui::opengl::GLShaderProgram>(vertex_shader, frag_shader);
    }
}

void OpenGLShapeRenderer::cleanup_gl() {}

void OpenGLShapeRenderer::render_shapes(
    const std::vector<Quad> &quads,
    const std::vector<Polygon> &polygons,
    const std::vector<Ellipse> &ellipses,
    const Imath::M44f &transform_window_to_viewport_space,
    const Imath::M44f &transform_viewport_to_image_space,
    float viewport_du_dx) {

    if (!shader_)
        init_gl();

    utility::JsonStore shader_params;

    shader_params["image_aspectratio"] = transform_window_to_viewport_space[1][1];

    for (int i = 0; i < quads.size(); ++i) {
        shader_params[fmt::format("quads[{}].tl", i)]       = quads[i].tl;
        shader_params[fmt::format("quads[{}].tr", i)]       = quads[i].tr;
        shader_params[fmt::format("quads[{}].br", i)]       = quads[i].br;
        shader_params[fmt::format("quads[{}].bl", i)]       = quads[i].bl;
        shader_params[fmt::format("quads[{}].color", i)]    = quads[i].colour;
        shader_params[fmt::format("quads[{}].softness", i)] = quads[i].softness / 500.0f;
        shader_params[fmt::format("quads[{}].opacity", i)]  = quads[i].opacity / 100.0f;
        shader_params[fmt::format("quads[{}].invert", i)]   = quads[i].invert ? -1.f : 1.f;
    }
    shader_params["quads_count"] = quads.size();

    for (int i = 0; i < polygons.size(); ++i) {
        for (int j = 0; j < polygons[i].points.size(); ++j) {
            shader_params[fmt::format("polygons[{}].points[{}]", i, j)] = polygons[i].points[j];
        }
        shader_params[fmt::format("polygons[{}].count", i)]    = polygons[i].points.size();
        shader_params[fmt::format("polygons[{}].color", i)]    = polygons[i].colour;
        shader_params[fmt::format("polygons[{}].softness", i)] = polygons[i].softness / 500.0f;
        shader_params[fmt::format("polygons[{}].opacity", i)]  = polygons[i].opacity / 100.0f;
        shader_params[fmt::format("polygons[{}].invert", i)] = polygons[i].invert ? -1.f : 1.f;
    }
    shader_params["polygons_count"] = polygons.size();

    for (int i = 0; i < ellipses.size(); ++i) {
        shader_params[fmt::format("ellipses[{}].center", i)]   = ellipses[i].center;
        shader_params[fmt::format("ellipses[{}].radius", i)]   = ellipses[i].radius;
        shader_params[fmt::format("ellipses[{}].angle", i)]    = ellipses[i].angle;
        shader_params[fmt::format("ellipses[{}].color", i)]    = ellipses[i].colour;
        shader_params[fmt::format("ellipses[{}].softness", i)] = ellipses[i].softness / 500.0f;
        shader_params[fmt::format("ellipses[{}].opacity", i)]  = ellipses[i].opacity / 100.0f;
        shader_params[fmt::format("ellipses[{}].invert", i)] = ellipses[i].invert ? -1.f : 1.f;
    }
    shader_params["ellipses_count"] = ellipses.size();

    shader_->use();
    shader_->set_shader_parameters(shader_params);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glEnable(GL_DEPTH_TEST);

    shader_->stop_using();
}