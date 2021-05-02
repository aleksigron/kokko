#pragma once

#include "yaml-cpp/yaml.h"

#include "Math/Vec3.hpp"

namespace YAML
{
template<>
struct convert<Vec3<float>>
{
    static Node encode(const Vec3<float>& rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        return node;
    }

    static bool decode(const Node& node, Vec3<float>& rhs)
    {
        if (!node.IsSequence() || node.size() != 3)
        {
            return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        return true;
    }
};
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Vec3f& v)
{
    out << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.y << YAML::EndSeq;
    return out;
}
