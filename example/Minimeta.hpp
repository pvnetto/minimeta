#pragma once

#include <stdint.h>
#include <type_traits>
#include <typeinfo>

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
namespace utils {
static constexpr uint64_t kFNV1aValue = 0xcbf29ce484222325;
static constexpr uint64_t kFNV1aPrime = 0x100000001b3;

inline constexpr uint64_t hash(char const *const str,
                               uint64_t const value = kFNV1aValue) noexcept {
  return (str[0] == '\0')
             ? value
             : hash(&str[1], (value ^ uint64_t(str[0])) * kFNV1aPrime);
}
} // namespace utils
} // namespace mmeta

namespace mmeta {

    template <typename T>
    struct is_serializable {
        static constexpr bool value = false;
    };


    class mmtype {
    public:
        mmtype(uint64_t size, uint64_t hash, const char* name) : 
            m_size(size), m_hash(hash), m_name(name) { }

        inline const char* name() const { return m_name; }
        inline uint64_t size() const { return m_size; }
        inline uint64_t hash() const { return m_hash; }

        void dump() const {
            printf("info: name => %s, size => %lli, hash => %lli\n", name(), size(), hash());
        }

    private:
        uint64_t m_size;
        uint64_t m_hash;
        const char* m_name;
    };


    class mmfield {
    public:
        inline mmtype const * type() const { return m_type; }
        inline char const * name() const { return m_name; }

    private:
        mmtype const * m_type;
        char const * m_name;
    };


    class mmclass : public mmtype {
    public:
    private:
        uint32_t m_numFields = 0;
        mmfield* m_fields = nullptr;
    };

    template <typename T, uint32_t NumFields>
    struct mmclass_storage {
        static constexpr uint32_t FieldCount = NumFields;
        mmfield Fields[NumFields];

        using ctor_lambda = void (*)(mmclass_storage*);
        mmclass_storage(ctor_lambda ctor) {
            ctor(this);
        }
    };

    template <typename T>
    std::enable_if_t<
        std::is_fundamental_v<T>, mmtype const *
    >
    type() {
        static mmtype data { sizeof(T), utils::hash(typeid(T).name()), typeid(T).name() };
        return &data;
    }

    template <typename T>
    mmclass const * metadata() {
        static mmclass data;
        return &data;
    }
}

#include "Generated.hpp"