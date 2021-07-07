#pragma once

#include "Minimeta.hpp"

class SERIALIZABLE CustomAccessors {
    META_OBJECT
public:
    float show;
    float INTERNAL dontShow;

private:
    float SERIALIZE showPrivate;
};

struct SERIALIZABLE MoreCustomAccessors {
    META_OBJECT

    float Show;
    float INTERNAL DontShow;
};

struct NotSerializable {
    META_OBJECT

    float A, B, C;
};

class AlsoNotSerializable {
    META_OBJECT
    
public:
    int a, b;

private:
    int c;
};

#include "Examples.generated.hpp"