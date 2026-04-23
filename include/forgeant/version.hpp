#ifndef FORGEANT_VERSION_HPP
#define FORGEANT_VERSION_HPP

#include <string_view>

namespace forgeant {

/// @brief Major version number of the forgeant library.
inline constexpr int VERSION_MAJOR = 0;
/// @brief Minor version number of the forgeant library.
inline constexpr int VERSION_MINOR = 2;
/// @brief Patch version number of the forgeant library.
inline constexpr int VERSION_PATCH = 0;

/// @brief Returns the full version as `"MAJOR.MINOR.PATCH"`.
std::string_view version_string() noexcept;

} // namespace forgeant

#endif // FORGEANT_VERSION_HPP
