#pragma once

#include "Minimeta.hpp"

struct SERIALIZABLE Vec3 {
    float X, Y, Z;
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