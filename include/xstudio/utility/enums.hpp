// SPDX-License-Identifier: Apache-2.0
#pragma once

#define MAKE_ENUM(name, start, ...) typedef enum { name##First=start, __VA_ARGS__ } name;\
inline static const std::vector<name> name##All = {name##First, __VA_ARGS__};\
inline static const std::string_view name##Strings = #__VA_ARGS__;

namespace xstudio::utility {
enum class TimeSourceMode { FIXED = 1, REMAPPED = 2, DYNAMIC = 3 };
enum NotificationType {
    NT_UNKNOWN = 0,
    NT_INFO,
    NT_WARN,
    NT_PROCESSING,
    NT_PROGRESS_RANGE,
    NT_PROGRESS_PERCENTAGE
};

} // namespace xstudio::utility
