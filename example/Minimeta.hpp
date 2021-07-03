#pragma once

#include <iostream>

#include <stdint.h>
#include <type_traits>
#include <typeinfo>

#include <string_view>
#include <array>


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
            std::cout << "type: name => " << name() << ", size => " << size() << ", hash " << hash() << "\n";
        }

    private:
        const uint64_t m_size;
        const uint64_t m_hash;
        const std::string_view m_name;
    };


    class mmfield {
    public:
        constexpr mmfield(mmtype type, std::string_view name) : m_type(type), m_name(name) { }

        inline constexpr mmtype type() const { return m_type; }
        inline constexpr std::string_view name() const { return m_name; }

    private:
        const mmtype m_type;
        const std::string_view m_name;
    };

    struct fieldseq {
        const mmfield *Ptr;
        size_t Size;
        
        constexpr fieldseq(const mmfield* first, size_t sz) : Ptr(first), Size(sz) {}

        constexpr const mmfield *begin() const { return Ptr; }
        constexpr const mmfield *end() const { return Ptr + Size; }
        constexpr size_t size() const { return Size; }
    };

    class mmclass {
    public:
        mmclass() = default;
        constexpr mmclass(fieldseq fields) : m_fields(fields) { }

        constexpr size_t field_count() const { return m_fields.size(); }
        
        void dump() const {
            std::cout  << "class: num_fields => " << field_count() << "\n";
        }
    private:
        const fieldseq m_fields;
    };

    template <typename T>
    struct mmclass_storage {
        static constexpr mmfield Fields[] {};
    };

    
    // ========================================================================-------
    // ======= Type traits
    // ========================================================================-------
    template <typename T>
    using class_type = std::enable_if_t<std::is_class_v<T>, mmclass>;

    template <typename T>
    struct is_serializable {
        static constexpr bool value = std::is_fundamental_v<T>;
    };

    template<typename T>
    inline constexpr bool is_serializable_v = is_serializable<T>::value;

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
    struct classmeta {
        static constexpr int FieldCount() { return sizeof(mmclass_storage<T>::Fields) / sizeof(mmfield); }

        static constexpr mmclass value = {
            { mmclass_storage<T>::Fields, FieldCount()  }
        };
    };

    template<typename T>
    inline constexpr class_type<T> classmeta_v = classmeta<T>::value;

    struct NotSerializable { int dkajshda; };

    struct teststruct {
        int a;
        char b;
        NotSerializable not;
    };

    template<>
    struct mmclass_storage<teststruct> {
        static constexpr char* FieldAName = "a";

        static constexpr mmfield Fields[] {
            { typemeta_v<int>, "a" },
            { typemeta_v<float>, "b" },
            { typemeta_v<NotSerializable>, "not" },
        };
    };
}

#include "Generated.hpp"