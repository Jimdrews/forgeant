#pragma once

#include <string_view>

namespace agentforge {

inline constexpr int VERSION_MAJOR = 0;
inline constexpr int VERSION_MINOR = 1;
inline constexpr int VERSION_PATCH = 0;

std::string_view version_string() noexcept;

} // namespace agentforge
