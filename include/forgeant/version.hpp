#ifndef FORGEANT_VERSION_HPP
#define FORGEANT_VERSION_HPP

#include <string_view>

namespace forgeant {

inline constexpr int VERSION_MAJOR = 0;
inline constexpr int VERSION_MINOR = 2;
inline constexpr int VERSION_PATCH = 0;

std::string_view version_string() noexcept;

} // namespace forgeant

#endif // FORGEANT_VERSION_HPP
