#pragma once

#include "ryml.hpp"
#include "c4/format.hpp"

#include "Math/Vec3.hpp"

template<class T>
bool from_chars(ryml::csubstr buf, Vec2<T>* v);

template<class T>
bool from_chars(ryml::csubstr buf, Vec3<T>* v);

template<class T>
size_t to_chars(ryml::substr buf, const Vec2<T>& v);

template<class T>
size_t to_chars(ryml::substr buf, const Vec3<T>& v);
