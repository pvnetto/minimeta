#ifndef __MMETA_IGNORE__

// This file was generated by minimeta. Don't touch it!!
#pragma once
#include "Minimeta.hpp"
class CustomAccessors;
namespace mmeta{
template <>
struct is_serializable<CustomAccessors> {
  static constexpr bool value = true;
};
}
struct MoreCustomAccessors;
namespace mmeta{
template <>
struct is_serializable<MoreCustomAccessors> {
  static constexpr bool value = true;
};
}
#endif