#pragma once

#include "Minimeta.hpp"

struct SERIALIZABLE Vec3 {
    float X = 0.f, Y = 0.f, Z = 0.f;
};

class SERIALIZABLE Player {
public:
    Vec3 m_position;

private:
    const char* name = "Paiva";
};


class Foo {
public:
    int a;

    void Bar();
};