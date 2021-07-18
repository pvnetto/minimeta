#pragma once

#ifndef __MMETA__
namespace mmeta {
    template <typename T>
    struct mmclass_storage;
}
#endif

#ifdef __MMETA__
#define SERIALIZABLE __attribute__((annotate("mm-type")))
#define SERIALIZE __attribute__((annotate("mm-add")))
#define INTERNAL __attribute__((annotate("mm-ignore")))
#define META_OBJECT 

#define static_assert(x, ...)
#else
#define SERIALIZABLE
#define SERIALIZE
#define INTERNAL
#define META_OBJECT \
    template <typename T> friend struct mmeta::mmclass_storage;
#endif