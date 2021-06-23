#pragma once

#include "Minimeta.h"

class SERIALIZABLE CustomAccessors {
public:
    float show;
    float INTERNAL dontShow;

private:
    float SERIALIZE showPrivate;
};

struct SERIALIZABLE MoreCustomAccessors {
    float Show;
    float INTERNAL DontShow;
};

struct NotSerializable {
    float A, B, C;
};

class AlsoNotSerializable {
public:
    int a, b;

private:
    int c;
};