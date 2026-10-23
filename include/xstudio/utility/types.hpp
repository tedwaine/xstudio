// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <caf/type_id.hpp>
#include <fmt/format.h>

namespace xstudio::utility {

struct ColourTriplet {

    ColourTriplet()                       = default;
    ColourTriplet(const ColourTriplet &o) = default;
    ColourTriplet(const float _r, const float _g, const float _b, const float _a=1.0f) : r(_r), g(_g), b(_b), a(_a) {}

    ColourTriplet &operator=(const ColourTriplet &o) = default;

    bool operator==(const ColourTriplet &o) const { return r == o.r && g == o.g && b == o.b && a == o.a; }
    bool operator!=(const ColourTriplet &o) const {
        return !(r == o.r && g == o.g && b == o.b && a == o.a);
    }

    float r = {0.0f}; // NOLINT
    float g = {0.0f}; // NOLINT
    float b = {0.0f}; // NOLINT
    float a = {1.0f}; // NOLINT

    [[nodiscard]] float red() const { return r; }
    [[nodiscard]] float green() const { return g; }
    [[nodiscard]] float blue() const { return b; }
    [[nodiscard]] float alpha() const { return a; }

    void set_red(const float _r) { r = _r; }
    void set_green(const float _g) { g = _g; }
    void set_blue(const float _b) { b = _b; }
    void set_alpha(const float _a) { a = _a; }

    friend std::string to_string(const ColourTriplet &value);

    template <class Inspector> friend bool inspect(Inspector &f, ColourTriplet &x) {
        return f.object(x).fields(f.field("r", x.r), f.field("g", x.g), f.field("b", x.b), f.field("a", x.a));
    }
};

inline std::string to_string(const ColourTriplet &c) {
    return fmt::format("ColourTriplet({}, {}, {}, {})", c.r, c.g, c.b, c.a);
}


} // namespace xstudio::utility
