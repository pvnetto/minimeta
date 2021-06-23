#pragma once

#include "Minimeta.h"

struct SERIALIZABLE Vec3 {
    float X, Y, Z;
};

class SERIALIZABLE Player {
public:
    Vec3 m_position;

private:
    const char* name = "Paiva";
};