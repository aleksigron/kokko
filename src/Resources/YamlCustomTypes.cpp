#include "Resources/YamlCustomTypes.hpp"

YAML::Emitter& operator<<(YAML::Emitter& out, const Vec3f& v)
{
    out << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
    return out;
}
