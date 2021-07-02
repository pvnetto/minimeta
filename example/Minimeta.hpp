#pragma once

#include <iostream>

#include <stdint.h>
#include <type_traits>
#include <typeinfo>

#include <string_view>


#ifdef __MMETA__
#define SERIALIZABLE __attribute__((annotate("mm-type")))
#define SERIALIZE __attribute__((annotate("mm-add")))
#define INTERNAL __attribute__((annotate("mm-ignore")))
#else
#define SERIALIZABLE
#define SERIALIZE
#define INTERNAL
#endif

// based on: https://github.com/Manu343726/ctti/blob/master/include/ctti/detail/pretty_function.hpp
#if defined(__clang__)
    #define MMETA_PRETTY_FUNCTION __PRETTY_FUNCTION__
    #define MMETA_NAME_PREFFIX "mmeta::utils::type_name<"
    #define MMETA_NAME_SUFFIX ">" 
#elif defined(__GNUC__) && !defined(__clang__)
    #define MMETA_PRETTY_FUNCTION __PRETTY_FUNCTION__
    #define MMETA_NAME_PREFFIX "mmeta::utils::type_name<"
    #define MMETA_NAME_SUFFIX ">" 
#elif defined(_MSC_VER)
    #define MMETA_PRETTY_FUNCTION __FUNCSIG__
    #define MMETA_NAME_PREFFIX "mmeta::utils::type_name<"
    #define MMETA_NAME_SUFFIX ">::prettified_name"
#else
    #error "No support for this compiler."
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

        inline constexpr uint64_t hash(std::string_view str) {
            return hash(str.data());
        }

        template <typename T>
        struct type_name {
          static constexpr std::string_view prettified_name() {
              constexpr std::string_view ugly_name = { MMETA_PRETTY_FUNCTION };

              constexpr std::string_view preffix = { MMETA_NAME_PREFFIX }; 
              constexpr const size_t preffixLoc = ugly_name.find(preffix);
              constexpr std::string_view padded_front = ugly_name.substr(preffixLoc + preffix.size());

              constexpr std::string_view suffix = { MMETA_NAME_SUFFIX }; 
              constexpr const size_t suffixLoc = padded_front.find(suffix);
              return padded_front.substr(0, suffixLoc);
          }

          static constexpr std::string_view name = prettified_name();
        };
    }
}

namespace mmeta {

    // ========================================================================-------
    // ======= Types
    // ========================================================================-------
    class mmtype {
    public:
        mmtype() = default;
        constexpr mmtype(const uint64_t size, const uint64_t hash, std::string_view name) : 
            m_size(size), m_hash(hash), m_name(name) { }

        inline constexpr std::string_view name() const { return m_name; }
        inline constexpr uint64_t size() const { return m_size; }
        inline constexpr uint64_t hash() const { return m_hash; }

        void dump() const {
            std::cout << "info: name => " << name() << ", size => " << size() << ", hash " << hash() << "\n";
        }

    private:
        const uint64_t m_size;
        const uint64_t m_hash;
        const std::string_view m_name;
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


    class mmclass {
    public:
        mmclass() = default;
        mmclass(const uint32_t numFields, mmfield* fields) :
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
    using class_type = std::enable_if_t<std::is_class_v<T>, mmclass const *>;

    template <typename T>
    struct is_serializable {
        static constexpr bool value = false;
    };

    template <typename T>
    class_type<T> get_class_meta() {
        static const mmclass data;
        return &data;
    }


    // ========================================================================-------
    // ======= Syntatic sugar
    // ========================================================================-------
    template<typename T>
    struct typemeta {
        static constexpr mmtype value = {
            sizeof(T),
            utils::hash(utils::type_name<T>::name),
            utils::type_name<T>::name };
    };

    template<typename T>
    inline constexpr mmtype typemeta_v = typemeta<T>::value;

    template<typename T>
    class_type<T> classmeta_v = get_class_meta<T>();

    struct teststruct {
        int a;
        char b;
    };

    template <>
    class_type<teststruct> get_class_meta<teststruct> () {
        auto ctor = [](mmclass_storage<teststruct, 2>* self) {
            // self
            self->Fields[0] = { &typemeta_v<int>, "a" };
            self->Fields[1] = { &typemeta_v<char>, "c" };
        };
        mmclass_storage<teststruct, 2> storage {ctor};
        
        static mmclass data {
            storage.FieldCount,
            storage.Fields
        };
        return &data;
    } 
}

#include "Generated.hpp"