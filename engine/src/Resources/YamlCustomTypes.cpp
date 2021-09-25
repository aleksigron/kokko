#include "Resources/YamlCustomTypes.hpp"

YAML::Emitter& operator<<(YAML::Emitter& out, const Vec3<float>& v)
{
    out << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Vec2<float>& v)
{
    out << YAML::Flow << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
    return out;
}
