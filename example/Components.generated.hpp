#ifndef __MMETA__

// This file was generated by minimeta. Don't touch it!!
#pragma once
#include "Minimeta.hpp"
struct Vec3;
namespace mmeta{
  template <>
  struct is_serializable<Vec3> : std::true_type {};

  MMCLASS_STORAGE(Vec3,MMFIELD_STORAGE(X),MMFIELD_STORAGE(Z),)}
class Player;
namespace mmeta{
  template <>
  struct is_serializable<Player> : std::true_type {};

  MMCLASS_STORAGE(Player,MMFIELD_STORAGE(m_id),MMFIELD_STORAGE(m_state),MMFIELD_STORAGE(m_position),)}
struct Transform;
namespace mmeta{
  template <>
  struct is_serializable<Transform> : std::true_type {};

  MMCLASS_STORAGE(Transform,MMFIELD_STORAGE(Position),MMFIELD_STORAGE(Rotation),)}
#endif