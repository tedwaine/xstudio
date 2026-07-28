// SPDX-License-Identifier: Apache-2.0

#include <limits>
#include <caf/actor_registry.hpp>

#include "xstudio/utility/helpers.hpp"
#include "xstudio/utility/string_helpers.hpp"

#include "grading.h"
#include "grading_colour_op.hpp"
#include "grading_mask_render_data.h"
#include "grading_common.h"
#include "grading_mask_renderer.h"

using namespace xstudio;
using namespace xstudio::bookmark;
using namespace xstudio::colour_pipeline;
using namespace xstudio::ui::viewport;

GradingColourOperator::GradingColourOperator(
    caf::actor_config &cfg, const utility::JsonStore &init_settings)
    : ColourOpPlugin(cfg, "GradingColourOperator", init_settings) {

    // ask plugin manager for the instance of the GradingTool plugin
    auto pm = system().registry().template get<caf::actor>(plugin_manager_registry);
    mail(plugin_manager::get_resident_atom_v, GradingTool::PLUGIN_UUID)
        .request(pm, infinite)
        .then(
            [=](caf::actor grading_tool) mutable {
                // ping the grading tool with a pointer to ourselves, so it can
                // send us updates on the 'bypass' attr. GradingTool of course has
                // the necessary message handler for this
                anon_mail("follow_bypass", caf::actor_cast<caf::actor>(this))
                    .send(grading_tool);
            },
            [=](caf::error &err) mutable {

            });
}

caf::message_handler GradingColourOperator::message_handler_extensions() {

    // here's our handler for the messages coming from the GradingTool about
    // the state of its 'bypass' attribute.
    return caf::message_handler(
               {[=](utility::event_atom, const std::string &desc, bool bypass) {
                   if (desc == "bypass") {
                       bypass_ = bypass;
                   }
               }})
        .or_else(ColourOpPlugin::message_handler_extensions());
}

ColourOperationDataPtr GradingColourOperator::colour_op_graphics_data(
    utility::UuidActor &media_source, const utility::JsonStore &media_source_colour_metadata) {

    // The result of this call will depend on the ocio config, as the grading op
    // depends on the lin to lon transform which generally varies per ocio config.

    // It's also possible that the result will depend on the ocio_context because
    // for some jobs the log to lin transform varies PER SHOT. To Be extra safe
    // we make a hash from the whole context data dictionary

    std::string hash_data;
    if (media_source_colour_metadata.contains("ocio_config")) {
        hash_data += media_source_colour_metadata.get_or("ocio_config", std::string(""));
    }
    if (media_source_colour_metadata.contains("ocio_context")) {
        hash_data += media_source_colour_metadata["ocio_context"].dump();
    }

    size_t hash = std::hash<std::string>{}(hash_data);
    auto p      = colour_op_data_cache_.find(hash);
    if (p != colour_op_data_cache_.end()) {
        return p->second;
    }

    colour_op_data_cache_[hash] = setup_shader_data(media_source_colour_metadata);
    return colour_op_data_cache_[hash];
}

utility::JsonStore
GradingColourOperator::update_shader_uniforms(const media_reader::ImageBufPtr &image) {

    utility::JsonStore uniforms_dict;
    uniforms_dict["grade_count"] = 0;
    uniforms_dict["tool_active"] = false;

    if (bypass_) {
        return uniforms_dict;
    }

    size_t grade_count = 0;
    auto active_grades = get_active_grades(image);
    for (const auto grade_info : active_grades) {

        auto grade_data = grade_info.data;

        std::string grade_str = fmt::format("grades[{}].", grade_count);

        uniforms_dict[grade_str + "grade_active"] = grade_info.grade_active;

        // We only support compositing_log as colour space conversion for now.
        // All other values will be treated as being the current colour space.
        uniforms_dict[grade_str + "color_space"] =
            grade_data->colour_space() == "compositing_log" ? 1 : 0;
        uniforms_dict[grade_str + "mask_active"] = !grade_data->mask().empty();

        // NOTE (Ted) ..
        // When grade_data->mask_editing() is true, the mask is drawn applied
        // to the image as a yellow mix - in other words the grade isn't applied.
        // but the mask is shown in transparent yellow.
        // This depends on the 'display_mode' attr in the plugin, but ther is
        // nowhere in the UI to control this. Therefore, I disable here because
        // otherwise I'm sometimes seeing the mask_editing() flag as true, seems
        // a bit random and it's undesired
        uniforms_dict[grade_str + "mask_editing"] = false;
        // Here's what we used to have here:
        // uniforms_dict[grade_str + "mask_editing"] = grade_data->mask_editing();

        uniforms_dict[grade_str + "slope"] = {
            "vec3",
            1,
            grade_data->grade().slope[0] * grade_data->grade().slope[3],
            grade_data->grade().slope[1] * grade_data->grade().slope[3],
            grade_data->grade().slope[2] * grade_data->grade().slope[3]};
        uniforms_dict[grade_str + "offset"] = {
            "vec3",
            1,
            grade_data->grade().offset[0] + grade_data->grade().offset[3],
            grade_data->grade().offset[1] + grade_data->grade().offset[3],
            grade_data->grade().offset[2] + grade_data->grade().offset[3]};
        uniforms_dict[grade_str + "power"] = {
            "vec3",
            1,
            grade_data->grade().power[0] * grade_data->grade().power[3],
            grade_data->grade().power[1] * grade_data->grade().power[3],
            grade_data->grade().power[2] * grade_data->grade().power[3]};
        uniforms_dict[grade_str + "sat"]      = grade_data->grade().sat;
        uniforms_dict[grade_str + "exposure"] = grade_data->grade().exposure;
        uniforms_dict[grade_str + "contrast"] = grade_data->grade().contrast;

        grade_count++;
    }

    if (grade_count) {
        uniforms_dict["grade_count"] = grade_count;
        uniforms_dict["tool_active"] = true;
    }

    // std::cerr << uniforms_dict.dump() << "\n";

    return uniforms_dict;
}

std::shared_ptr<ColourOperationData> GradingColourOperator::setup_shader_data(
    const utility::JsonStore &media_source_colour_metadata) {

    auto colour_op_data =
        std::make_shared<ColourOperationData>(ColourOperationData(PLUGIN_UUID, "Grade OP"));

    std::vector<ColourLUTPtr> &fs_luts = colour_op_data->luts_;

    auto lin_to_log_shader_desc = setup_ocio_shader(
        "OCIOLinToLog",
        "ocio_lin_to_log",
        media_source_colour_metadata,
        "scene_linear",
        "compositing_log");

    auto lin_to_log_luts = setup_ocio_textures(lin_to_log_shader_desc);
    fs_luts.insert(fs_luts.end(), lin_to_log_luts.begin(), lin_to_log_luts.end());

    auto log_to_lin_shader_desc = setup_ocio_shader(
        "OCIOLogToLin",
        "ocio_log_to_lin",
        media_source_colour_metadata,
        "compositing_log",
        "scene_linear");

    auto log_to_lin_luts = setup_ocio_textures(log_to_lin_shader_desc);
    fs_luts.insert(fs_luts.end(), log_to_lin_luts.begin(), log_to_lin_luts.end());

    size_t hash;
    gradingop_shader_ = GradingMaskRenderer::make_mask_shader(lin_to_log_shader_desc, log_to_lin_shader_desc, hash);

    colour_op_data->shader_ = gradingop_shader_;
    colour_op_data->set_cache_id(fmt::format("{}", hash));
    for (const auto &lut : fs_luts) {
        colour_op_data->set_cache_id(
            colour_op_data->cache_id() + fmt::format("{}", lut->cache_id()));
    }
    return colour_op_data;
}

plugin::GPUPreDrawHookPtr
GradingColourOperator::make_pre_draw_gpu_hook(const std::string &viewport_name) {
    return std::make_shared<GradingMaskRenderer>(viewport_name);
}

OCIO::ConstGpuShaderDescRcPtr GradingColourOperator::setup_ocio_shader(
    const std::string &function_name,
    const std::string &resource_prefix,
    const utility::JsonStore &metadata,
    const std::string &src,
    const std::string &dst) {

    auto desc = OCIO::GpuShaderDesc::CreateShaderDesc();
    desc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_4_0);
    desc->setFunctionName(function_name.c_str());
    desc->setResourcePrefix(resource_prefix.c_str());

    try {
        const std::string config_name = metadata.get_or("ocio_config", std::string(""));
        auto config                   = OCIO::Config::CreateFromFile(config_name.c_str());

        auto context = config->getCurrentContext()->createEditableCopy();
        if (metadata.contains("ocio_context")) {
            if (metadata["ocio_context"].is_object()) {
                for (auto &item : metadata["ocio_context"].items()) {
                    context->setStringVar(
                        item.key().c_str(), std::string(item.value()).c_str());
                }
            }
        }

        auto gpu =
            config->getProcessor(context, src.c_str(), dst.c_str())->getDefaultGPUProcessor();
        gpu->extractGpuShaderInfo(desc);
        return desc;
    } catch (const OCIO::Exception &ex) {
        auto config = OCIO::Config::CreateRaw();
        auto gpu    = config->getProcessor("raw", "raw")->getDefaultGPUProcessor();
        gpu->extractGpuShaderInfo(desc);
        return desc;
    }
}

std::vector<ColourLUTPtr>
GradingColourOperator::setup_ocio_textures(OCIO::ConstGpuShaderDescRcPtr &shader) {

    std::vector<ColourLUTPtr> luts;

    // Process 3D LUTs
    const unsigned max_texture_3D = shader->getNum3DTextures();
    for (unsigned idx = 0; idx < max_texture_3D; ++idx) {
        const char *textureName           = nullptr;
        const char *samplerName           = nullptr;
        unsigned edgelen                  = 0;
        OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;

        shader->get3DTexture(idx, textureName, samplerName, edgelen, interpolation);
        if (!textureName || !*textureName || !samplerName || !*samplerName || edgelen == 0) {
            throw std::runtime_error(
                "OCIO::ShaderDesc::get3DTexture - The texture data is corrupted");
        }

        const float *ocio_lut_data = nullptr;
        shader->get3DTextureValues(idx, ocio_lut_data);
        if (!ocio_lut_data) {
            throw std::runtime_error(
                "OCIO::ShaderDesc::get3DTextureValues - The texture values are missing");
        }

        auto xs_dtype    = LUTDescriptor::FLOAT32;
        auto xs_channels = LUTDescriptor::RGB;
        auto xs_interp   = interpolation == OCIO::INTERP_LINEAR ? LUTDescriptor::LINEAR
                                                                : LUTDescriptor::NEAREST;
        auto xs_lut      = std::make_shared<ColourLUT>(
            LUTDescriptor::Create3DLUT(edgelen, xs_dtype, xs_channels, xs_interp), samplerName);

        const int channels          = 3;
        const std::size_t data_size = edgelen * edgelen * edgelen * channels * sizeof(float);
        auto *xs_lut_data           = (float *)xs_lut->writeable_data();
        std::memcpy(xs_lut_data, ocio_lut_data, data_size);

        xs_lut->update_content_hash();
        luts.push_back(xs_lut);
    }

    // Process 1D LUTs
    const unsigned max_texture_2D = shader->getNumTextures();
    for (unsigned idx = 0; idx < max_texture_2D; ++idx) {
        const char *textureName                  = nullptr;
        const char *samplerName                  = nullptr;
        unsigned width                           = 0;
        unsigned height                          = 0;
        OCIO::GpuShaderDesc::TextureType channel = OCIO::GpuShaderDesc::TEXTURE_RGB_CHANNEL;
        OCIO::Interpolation interpolation        = OCIO::INTERP_LINEAR;
        bool is2DTexture                         = false;

#if OCIO_VERSION_HEX >= 0x02030000
        OCIO::GpuShaderDesc::TextureDimensions dimensions = OCIO::GpuShaderDesc::TEXTURE_1D;
        shader->getTexture(
            idx, textureName, samplerName, width, height, channel, dimensions, interpolation);
        is2DTexture = dimensions == OCIO::GpuShaderDesc::TEXTURE_2D;
#else
        shader->getTexture(
            idx, textureName, samplerName, width, height, channel, interpolation);
        is2DTexture = height > 1;
#endif
        if (!textureName || !*textureName || !samplerName || !*samplerName || width == 0) {
            throw std::runtime_error(
                "OCIO::ShaderDesc::getTexture - The texture data is corrupted");
        }

        const float *ocio_lut_data = nullptr;
        shader->getTextureValues(idx, ocio_lut_data);
        if (!ocio_lut_data) {
            throw std::runtime_error(
                "OCIO::ShaderDesc::getTextureValues - The texture values are missing");
        }

        auto xs_dtype    = LUTDescriptor::FLOAT32;
        auto xs_channels = channel == OCIO::GpuShaderCreator::TEXTURE_RED_CHANNEL
                               ? LUTDescriptor::RED
                               : LUTDescriptor::RGB;
        auto xs_interp   = interpolation == OCIO::INTERP_LINEAR ? LUTDescriptor::LINEAR
                                                                : LUTDescriptor::NEAREST;
        auto xs_lut      = std::make_shared<ColourLUT>(
            is2DTexture
                ? LUTDescriptor::Create2DLUT(width, height, xs_dtype, xs_channels, xs_interp)
                : LUTDescriptor::Create1DLUT(width, xs_dtype, xs_channels, xs_interp),
            samplerName);

        const int channels = channel == OCIO::GpuShaderCreator::TEXTURE_RED_CHANNEL ? 1 : 3;
        const std::size_t data_size = width * height * channels * sizeof(float);
        auto *xs_lut_data           = (float *)xs_lut->writeable_data();
        std::memcpy(xs_lut_data, ocio_lut_data, data_size);

        xs_lut->update_content_hash();
        luts.push_back(xs_lut);
    }

    return luts;
}
