#pragma once

#include "Minimeta.hpp"

struct SERIALIZABLE Vec3 {
    float X = 0.f;
    float Y = 0.f;
    float Z = 0.f;

    void Dump() {
        printf("X => %f, Y => %f, Z => %f\n", X, Y, Z);
    }

    META_OBJECT
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

    void SetPosition(Vec3 position) { m_position = position; }
    void SetName(const std::string& name) { m_name = name; }

    void Dump() {
        printf("ID => %i, Name => %s, X => %f, Y => %f, Z => %f\n",
            m_id, m_name.c_str(), m_position.X, m_position.Y, m_position.Z);
    }

private:
    Vec3 SERIALIZE m_position { 0.f, 0.f, 0.f };
    std::string SERIALIZE m_name = "Default";

    META_OBJECT
};

struct SERIALIZABLE Transform {
    Vec3 Position { 0.f, 0.f, 0.f };
    float Rotation = 0.f;

    META_OBJECT
};

#include "Components.generated.hpp"