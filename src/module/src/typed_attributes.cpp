// SPDX-License-Identifier: Apache-2.0
#include "xstudio/module/typed_attributes.hpp"

using namespace xstudio::module;
using namespace xstudio;

FloatAttribute::FloatAttribute(
    const std::string &title,
    const std::string &abbr_title,
    const float value,
    const float float_min,
    const float float_max,
    const float float_step,
    const int float_display_decimals,
    const float fscrub_sensitivity)
    : TypeAttribute<float>(title, abbr_title, value, type_name(Attribute::FloatAttribute)) {
    set_role_data(FloatScrubMin, float_min);
    set_role_data(FloatScrubMax, float_max);
    set_role_data(FloatScrubStep, float_step);
    set_role_data(FloatScrubSensitivity, fscrub_sensitivity);
    set_role_data(FloatDisplayDecimals, float_display_decimals);
}

StringChoiceAttribute::StringChoiceAttribute(
    const std::string &title,
    const std::string &abbr_title,
    const std::string &value,
    const std::vector<std::string> &options,
    const std::vector<std::string> &abbr_options)
    : TypeAttribute<std::string>(
          title, abbr_title, value, type_name(Attribute::StringChoiceAttribute)) {
    set_role_data(StringChoices, options);
    set_role_data(AbbrStringChoices, abbr_options);
}

BooleanAttribute::BooleanAttribute(
    const std::string &title, const std::string &abbr_title, const bool value)
    : TypeAttribute<bool>(title, abbr_title, value, type_name(Attribute::BooleanAttribute)) {}

StringAttribute::StringAttribute(
    const std::string &title, const std::string &abbr_title, const std::string &value)
    : TypeAttribute<std::string>(
          title, abbr_title, value, type_name(Attribute::StringAttribute)) {}

QmlCodeAttribute::QmlCodeAttribute(const std::string &title, const std::string &code)
    : TypeAttribute<std::string>(title, title, "", type_name(Attribute::QmlCodeAttribute)) {
    set_role_data(QmlCode, code);
}

IntegerAttribute::IntegerAttribute(
    const std::string &title,
    const std::string &abbr_title,
    const int64_t value,
    const int64_t int_min,
    const int64_t int_max)
    : TypeAttribute<int64_t>(title, abbr_title, value, type_name(Attribute::IntegerAttribute)) {
    set_role_data(IntegerMin, int_min);
    set_role_data(IntegerMax, int_max);
}

ColourAttribute::ColourAttribute(
    const std::string &title,
    const std::string &abbr_title,
    const utility::ColourTriplet &value)
    : TypeAttribute<utility::ColourTriplet>(
          title, abbr_title, value, type_name(Attribute::ColourAttribute)) {}

Vec4fAttribute::Vec4fAttribute(
    const std::string &title, const std::string &abbr_title, const Imath::V4f &value)
    : TypeAttribute<Imath::V4f>(
          title, abbr_title, value, type_name(Attribute::Vec4fAttribute)) {}

JsonAttribute::JsonAttribute(
    const std::string &title, const std::string &abbr_title, const nlohmann::json &value)
    : TypeAttribute<nlohmann::json>(
          title, abbr_title, value, type_name(Attribute::JsonAttribute)) {}

IntegerVecAttribute::IntegerVecAttribute(
    const std::string &title, const std::string &abbr_title, const std::vector<int> &value)
    : TypeAttribute<std::vector<int>>(
          title, abbr_title, value, type_name(Attribute::IntegerAttribute)) {}

FloatVectorAttribute::FloatVectorAttribute(
    const std::string &title, const std::string &abbr_title, const std::vector<float> &value)
    : TypeAttribute<std::vector<float>>(
          title, abbr_title, value, type_name(Attribute::FloatVectorAttribute)) {}
