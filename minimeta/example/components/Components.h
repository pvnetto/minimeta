#pragma once

#include <string>
#include <vector>

#include <mmeta/annotations.h>

namespace Math {
    struct SERIALIZABLE Vec3 {
        float X = 0.f;
        float Y = 0.f;
        float Z = 0.f;

        void Dump() { printf("X => %f, Y => %f, Z => %f\n", X, Y, Z); }
    };
} // namespace Components

struct SERIALIZABLE ColorRGB {
    float R = 0.f;
    float G = 0.f;
    float B = 0.f;
};

struct PlayerState {
    int State = 10;
    float Points = 300.f;
};

class SERIALIZABLE Player {
public:
    int m_id = 0;
    PlayerState m_state;
    std::vector<int> m_integers;
    std::vector<std::vector<float>> m_nested;
    std::vector<Math::Vec3> m_targets;

    void SetPosition(Math::Vec3 position) { m_position = position; }
    void SetName(const std::string &name) { m_name = name; }

    void Dump() {
    printf("ID => %i, Name => %s, X => %f, Y => %f, Z => %f\n", m_id,
            m_name.c_str(), m_position.X, m_position.Y, m_position.Z);
    }

private:
    Math::Vec3 SERIALIZE m_position{0.f, 0.f, 0.f};
    std::string SERIALIZE m_name = "Default";

    META_OBJECT
};

struct SERIALIZABLE Transform {
    Math::Vec3 Position{0.f, 0.f, 0.f};
    float Rotation = 0.f;
};

#include "Components.generated.hpp"