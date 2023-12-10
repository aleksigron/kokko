#include "Resources/YamlCustomTypes.hpp"

template<class T>
bool from_chars(ryml::csubstr buf, Vec2<T>* v)
{
    size_t ret = ryml::unformat(buf, "({},{})", v->x, v->y);
    return ret != ryml::yml::npos;
}

template bool from_chars<float>(ryml::csubstr buf, Vec2<float>* v);

template<class T>
bool from_chars(ryml::csubstr buf, Vec3<T>* v)
{
    size_t ret = ryml::unformat(buf, "({},{},{})", v->x, v->y, v->z);
    return ret != ryml::yml::npos;
}

template bool from_chars<float>(ryml::csubstr buf, Vec3<float>* v);

template<class T>
size_t to_chars(ryml::substr buf, const Vec2<T>& v)
{
    return ryml::format(buf, "({},{})", v.x, v.y);
}

template size_t to_chars<float>(ryml::substr buf, const Vec2<float>& v);

template<class T>
size_t to_chars(ryml::substr buf, const Vec3<T>& v)
{
    return ryml::format(buf, "({},{},{})", v.x, v.y, v.z);
}

template size_t to_chars<float>(ryml::substr buf, const Vec3<float>& v);
