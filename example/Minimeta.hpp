#pragma once

#ifdef __MMETA__
#define SERIALIZABLE __attribute__((annotate("mm-type")))
#define SERIALIZE __attribute__((annotate("mm-add")))
#define INTERNAL __attribute__((annotate("mm-ignore")))
#else
#define SERIALIZABLE
#define SERIALIZE
#define INTERNAL
#endif


namespace mmeta {
    template <typename T>
    struct is_serializable {
        static constexpr bool value = false;
    };
}

#include "Generated.hpp"