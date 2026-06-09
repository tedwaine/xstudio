// SPDX-License-Identifier: Apache-2.0
#include <exception>
#include <filesystem>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "ppm_pixel_unpack_shader.hpp"
#include "xstudio/media_reader/media_reader.hpp"
#include "xstudio/media/media_error.hpp"
#include "xstudio/utility/helpers.hpp"
#include "xstudio/media_reader/media_reader.hpp"

namespace fs = std::filesystem;

using namespace xstudio::media_reader;
using namespace xstudio::utility;
using namespace xstudio;

namespace {
    static utility::Uuid myshader_uuid{"1c9259fc-46a5-11ea-9de9-989096adb429"};
    static ui::viewport::GPUShaderPtr ppm_shader(new PPMPixelUnpackShader(myshader_uuid));
}

namespace xstudio::media_reader {
class PPMMediaReader : public MediaReader {
  public:
    PPMMediaReader(const utility::JsonStore &prefs = utility::JsonStore())
        : MediaReader("PPM", prefs) {
        try {
            supported_ = global_store::preference_value<utility::JsonStore>(
                prefs, "/plugin/media_reader/ppm/supported");
        } catch (const std::exception &e) {
            spdlog::warn("{} {}", __PRETTY_FUNCTION__, e.what());
        }
    }
    virtual ~PPMMediaReader() = default;

    ImageBufPtr image(const media::AVFrameID &mptr) override;
    MRCertainty
    supported(const caf::uri &uri, const std::array<uint8_t, 16> &signature) override;
    [[nodiscard]] std::vector<std::string> supported_extensions() const override;

    // media::MediaDetail detail(const caf::uri &uri) const override;
    [[nodiscard]] utility::Uuid plugin_uuid() const override { return PLUGIN_UUID; }

    static inline const utility::Uuid PLUGIN_UUID = utility::Uuid("c0465f96-901a-42bc-875b-ecf30f1eef14");

  private:
    utility::JsonStore supported_;
};
} // namespace xstudio::media_reader

ImageBufPtr PPMMediaReader::image(const media::AVFrameID &mptr) {
    ImageBufPtr buf;

    std::ifstream inp(uri_to_posix_path(mptr.uri()), std::ios::in | std::ios::binary);
    if (inp.is_open()) {
        size_t width;
        size_t height;
        size_t max_col_val;

        std::string line;

        std::getline(inp, line);
        if (line != "P6")
            throw media_corrupt_error(
                "Error. Unrecognized file format." + to_string(mptr.uri()));

        std::getline(inp, line);
        while (line[0] == '#') {
            std::getline(inp, line);
        }

        std::stringstream dimensions(line);
        try {
            dimensions >> width;
            dimensions >> height;
        } catch (std::exception &e) {
            throw media_corrupt_error(std::string("Header file format error. ") + e.what());
        }

        std::getline(inp, line);
        std::stringstream max_val(line);
        try {
            max_val >> max_col_val;
        } catch (std::exception &e) {
            throw media_corrupt_error(std::string("Header file format error. ") + e.what());
        }

        size_t size           = width * height;
        int bytes_per_channel = (max_col_val == 65535 ? 2 : 1);
        int bytes_per_pixel   = 3 * bytes_per_channel;

        JsonStore jsn;
        jsn["bytes_per_channel"] = bytes_per_channel;
        jsn["width"]             = width;
        jsn["height"]            = height;

        buf.reset(new ImageBuffer(myshader_uuid, jsn));
        buf->allocate(size * bytes_per_pixel);
        buf->set_shader(ppm_shader);
        buf->set_image_dimensions(Imath::V2i(width, height));

        byte *buffer = buf->buffer();
        inp.read((char *)buffer, size * bytes_per_pixel);
        inp.close();

    } else {
        throw media_unreadable_error("Unable to open " + to_string(mptr.uri()));
    }

    return buf;
}

std::vector<std::string> PPMMediaReader::supported_extensions() const {
    auto result = std::vector<std::string>();

    for (const auto &i : supported_.items()) {
        if (from_string(i.value()) != MRC_NO)
            result.push_back(i.key());
    }

    return result;
}

MRCertainty PPMMediaReader::supported(const caf::uri &uri, const std::array<uint8_t, 16> &sig) {
    auto result = MRC_NO;
    // we ignore the signature..
    // we cover so MANY...
    // but we're pretty good at movs..
    if (sig[0] == 'P' and sig[1] == '6' and sig[2] == '\n') {
        fs::path p(uri_to_posix_path(uri));

#ifdef _WIN32
        std::string ext = ltrim_char(to_upper_path(p.extension()), '.');
#else
        std::string ext = ltrim_char(to_upper(p.extension().string()), '.');
#endif

        try {
            result = from_string(supported_.value(ext, "MRC_NO"));
        } catch (const std::exception &err) {
            spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
        }
    }

    return result;
}

XSTUDIO_PLUGIN_DECLARE_BEGIN()

XSTUDIO_REGISTER_MEDIA_READER_PLUGIN(
    PPMMediaReader,
    PPMMediaReader::PLUGIN_UUID,
    PPMMediaReader,
    xStudio,
    PPM Media Reader,
    1.0.0)

XSTUDIO_PLUGIN_DECLARE_END()
