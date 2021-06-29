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
        // hash is based on: https://github.com/Leandros/metareflect
        static constexpr uint64_t kFNV1aValue = 0xcbf29ce484222325;
        static constexpr uint64_t kFNV1aPrime = 0x100000001b3;

        inline constexpr uint64_t hash(char const *const str,
                                    uint64_t const value = kFNV1aValue) noexcept {
        return (str[0] == '\0')
                    ? value
                    : hash(&str[1], (value ^ uint64_t(str[0])) * kFNV1aPrime);
        }
    }
}

namespace mmeta {

    // ========================================================================-------
    // ======= Types
    // ========================================================================-------
    class mmtype {
    public:
        mmtype() = default;
        mmtype(const uint64_t size, const uint64_t hash, const char* name) : 
            m_size(size), m_hash(hash), m_name(name) { }

        inline const char* name() const { return m_name; }
        inline uint64_t size() const { return m_size; }
        inline uint64_t hash() const { return m_hash; }

        void dump() const {
            printf("info: name => %s, size => %lli, hash => %lli\n", name(), size(), hash());
        }

    private:
        uint64_t m_size = 0;
        uint64_t m_hash = 0;
        const char* m_name = "";
    };


    class mmfield {
    public:
        mmfield() = default;
        mmfield(mmtype const* type, char const* name) : m_type(type), m_name(name) { }

        inline mmtype const * type() const { return m_type; }
        inline char const * name() const { return m_name; }

    private:
        mmtype const * m_type;
        char const * m_name;
    };


    class mmclass : public mmtype {
    public:
        mmclass() = default;
        mmclass(uint32_t numFields, mmfield* fields) :
            m_numFields(numFields), m_fields(fields) { }

    private:
        uint32_t m_numFields = 0;
        mmfield* m_fields = nullptr;
    };

    template <typename T, uint32_t NumFields>
    struct mmclass_storage {
        static constexpr uint32_t FieldCount = NumFields;
        mmfield Fields[NumFields];

        using ctor_lambda = void (mmclass_storage*);
        mmclass_storage(ctor_lambda ctor) {
            ctor(this);
        }
    };

    
    // ========================================================================-------
    // ======= Type traits
    // ========================================================================-------
    template <typename T>
    using primitive_type = std::enable_if_t<std::is_fundamental_v<T>, mmtype const *>;
    template <typename T>
    using user_defined_type = std::enable_if_t<!std::is_fundamental_v<T>, mmtype const *>;

    template <typename T>
    struct is_serializable {
        static constexpr bool value = false;
    };

    template <typename T>
    mmclass const * metadata() {
        static const mmclass data;
        return &data;
    }

    template <typename T>
    primitive_type<T> get_type_meta() {
        static const mmtype data { sizeof(T), utils::hash(typeid(T).name()), typeid(T).name() };
        return &data;
    }

    template <typename T>
    user_defined_type<T> get_type_meta() {
        return metadata<T>();
    }


    // ========================================================================-------
    // ======= Syntatic sugar
    // ========================================================================-------
    template <typename T>
    struct typemeta {
        static mmtype const * value;
    };

    template<typename T>
    mmtype const* typemeta<T>::value = get_type_meta<T>();

    template<typename T>
    mmtype const* typemeta_v = typemeta<T>::value;
    
    
    template <typename T>
    struct classmeta {
        static mmclass const * value;
    };

    template<typename T>
    mmclass const* classmeta<T>::value = metadata<T>();

    template<typename T>
    mmclass const* classmeta_v = classmeta<T>::value;
}

#include "Generated.hpp"