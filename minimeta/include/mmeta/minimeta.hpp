#pragma once

#include <iostream>
#include <stdint.h>
#include <type_traits>
#include <typeinfo>
#include <cassert>
#include <string_view>
#include <vector>
#include <utility>
#include <sstream>

#include <yaml-cpp/yaml.h>

#include "annotations.h"

// source: https://github.com/Manu343726/ctti/blob/master/include/ctti/detail/pretty_function.hpp
#if defined(__clang__)
    #define MMETA_PRETTY_FUNCTION __PRETTY_FUNCTION__
    #define MMETA_NAME_PREFFIX "[T = "
    #define MMETA_NAME_SUFFIX "]"
#elif defined(__GNUC__) && !defined(__clang__)
    #define MMETA_PRETTY_FUNCTION __PRETTY_FUNCTION__
    #define MMETA_NAME_PREFFIX "[with T = "
    #define MMETA_NAME_SUFFIX "]" 
#elif defined(_MSC_VER)
    #define MMETA_PRETTY_FUNCTION __FUNCSIG__
    #define MMETA_NAME_PREFFIX "mmeta::utils::type_name<"
    #define MMETA_NAME_SUFFIX ">::prettified_name"
#else
    #error "No support for this compiler."
#endif

#define BIT(x) (1 << x)

namespace mmeta {
    using hash_type = uint64_t;

    namespace utils {
        // source: https://github.com/Leandros/metareflect
        static constexpr hash_type kFNV1aValue = 0xcbf29ce484222325;
        static constexpr hash_type kFNV1aPrime = 0x100000001b3;

        inline constexpr hash_type hash(char const *const str,
                                    hash_type const value = kFNV1aValue) noexcept {
        return (str[0] == '\0')
                    ? value
                    : hash(&str[1], (value ^ hash_type(str[0])) * kFNV1aPrime);
        }

        inline constexpr hash_type hash(std::string_view str) {
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
    
    using binary_buffer = std::stringstream;        // Buffer used by client for both read/write operations
    using binary_buffer_type = binary_buffer::char_type;
    using binary_buffer_write = std::ostream;
    using binary_buffer_read = std::istream;

    class mmfield;

    struct mmactions {
        using ReadFn = void (*)(const mmfield*, binary_buffer_read&, void *);
        using WriteFn = void (*)(const mmfield*, const void *, binary_buffer_write&);

        using WriteYAMLFn = void (*)(const mmfield*, const void*, YAML::Node&);

        constexpr mmactions(const ReadFn readFn, const WriteFn writeFn, const WriteYAMLFn writeYamlFn) :
            Read(readFn), Write(writeFn), WriteYAML(writeYamlFn) {}

        const ReadFn Read;
        const WriteFn Write;
        const WriteYAMLFn WriteYAML;
    };

    class mmtype {
    public:
        constexpr mmtype(const size_t size, const uint64_t hash, std::string_view name, const mmactions actions) : 
            m_size(size),
            m_hash(hash),
            m_name(name),
            m_actions(actions) {}

        inline constexpr std::string_view name() const { return m_name; }
        inline constexpr size_t size() const { return m_size; }
        inline constexpr uint64_t hash() const { return m_hash; }
        inline constexpr mmactions actions() const { return m_actions; }

        void dump() const {
            std::cout << "type: name => " << name() << ", size => " << size() << ", hash " << hash() << "\n";
        }

    private:
        const size_t m_size;
        const uint64_t m_hash;
        const std::string_view m_name;
        const mmactions m_actions;
    };

    class mmfield {
    public:
        constexpr mmfield(mmtype type, std::string_view name, size_t offset) : m_type(type), m_name(name), m_offset(offset) { }

        inline constexpr mmtype type() const { return m_type; }
        inline constexpr std::string_view name() const { return m_name; }
        inline constexpr uint64_t hash() const { return m_type.hash(); }

        const void * get_pointer_from(const void * src) const {
            return static_cast<const binary_buffer_type*>(src) + m_offset;
        }

        void * get_pointer_from(void * src) const {
            return static_cast<binary_buffer_type*>(src) + m_offset;
        }

        template <typename T>
        T get_as(void const * src) const {
            assert(typemeta_v<T>.hash() == m_type.hash() && ">> ERROR: Trying to get field using wrong type.");
            T inst;
            memcpy(&inst, (binary_buffer_type *)src + m_offset, m_type.size());
            return inst;
        }

    private:
        const mmtype m_type;
        const std::string_view m_name;
        const size_t m_offset;
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
        constexpr mmclass(fieldseq fields, hash_type version) : m_fields(fields), m_version(version) {}

        constexpr size_t field_count() const { return m_fields.size(); }

        constexpr fieldseq fields() const { return m_fields; }

        constexpr hash_type version() const { return m_version; }
        
        void dump() const {
            std::cout  << "class: num_fields => " << field_count() << "\n";
        }
    private:
        const fieldseq m_fields;
        const hash_type m_version;
    };

    template <typename T>
    struct mmclass_storage {
        static constexpr size_t field_count() { return 0; }
        static constexpr fieldseq fields() { return { nullptr, 0 }; }
        static constexpr hash_type version() { return 0; }
    };

    
    // ========================================================================-------
    // ======= Type traits
    // ========================================================================-------
    
    template <hash_type>
    struct hashed_type {
        struct notype {};
        typedef notype value_type;
    };

    template <hash_type Hash>
    using hashed_type_t = typename hashed_type<Hash>::value_type;

    template <typename T>
    struct is_hashed_type {
        static constexpr bool value = std::is_same_v<T, hashed_type_t<utils::hash(utils::type_name<T>::name)>>;
    };

#ifdef __MMETA__
    template <typename T>
    inline constexpr bool is_hashed_type_v = std::is_class_v<T>;
#else
    template <typename T>
    inline constexpr bool is_hashed_type_v = is_hashed_type<T>::value;
#endif

    template <typename T>
    using class_type = std::enable_if_t<is_hashed_type_v<T>, mmclass>;

    template <typename T>
    constexpr mmactions type_actions();

    template<typename T>
    inline constexpr mmtype typemeta_v = {
        sizeof(T),
        utils::hash(utils::type_name<T>::name),
        utils::type_name<T>::name, type_actions<T>()};

    template<typename T>
    inline constexpr class_type<T> classmeta_v = {
        mmclass_storage<T>::fields(), mmclass_storage<T>::version()};

    template<class T>
    struct is_vector : std::false_type {};

    template<typename T>
    struct is_vector<std::vector<T>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_vector_v = is_vector<T>::value;

    template<typename T>
    inline constexpr bool is_string_v = std::is_same_v<T, std::string>;

    template <typename, class = void>
    struct is_defined : std::false_type {};
    
    template <typename T>
    struct is_defined<T,
        std::enable_if_t<std::is_object<T>::value &&
                        !std::is_pointer<T>::value &&
                        (sizeof(T) > 0)
        >
    > : std::true_type { };

    template <typename T>
    inline constexpr bool is_defined_v = is_defined<T>::value;

#ifdef __MMETA__
    template <typename T>
    struct is_serializable : std::true_type {};
#else
    template <typename T>
    struct is_serializable {
        static_assert(is_defined<T>::value && "is_serializable isn't really useful with forward declared types.");
        static constexpr bool value = std::is_fundamental_v<T> || is_hashed_type_v<T>;
    };

    template <typename T>
    struct is_serializable<std::vector<T>> {
        static constexpr bool value = is_serializable<T>::value;
    };

    template <>
    struct is_serializable<std::string> {
        static constexpr bool value = true;
    };
#endif

    template<typename T>
    inline constexpr bool is_serializable_v = is_serializable<T>::value;

    // FIXME: Move to some kind of utility part of the code
    template <typename T, typename Fn, std::size_t... I, typename... Args>
    constexpr std::enable_if_t<is_hashed_type_v<T>>
    for_each_field_impl(std::index_sequence<I...>, Fn&& fn, Args&... args) {
        ((fn(mmclass_storage<T>::Fields[I], args...)), ...);
    }

    template <typename T, typename Fn, typename... Args>
    constexpr std::enable_if_t<is_hashed_type_v<T>>
    for_each_field(Fn&& fn, Args&... args) {
        for_each_field_impl<T>(std::make_index_sequence<mmclass_storage<T>::field_count()>(), fn, args...);
    }

    template <typename T, std::size_t... I>
    static constexpr std::enable_if_t<is_hashed_type_v<T>, hash_type>
    combine_hashes(hash_type h, std::index_sequence<I...>) {
        auto mix_hash = [&](std::string_view other) { h = utils::hash(other.data(), h); };
        ((mix_hash(mmclass_storage<T>::Fields[I].type().name())), ...);
        return h;
    }

    template <typename T>
    static constexpr std::enable_if_t<is_hashed_type_v<T>, hash_type>
    class_version() {
        return combine_hashes<T>(
            typemeta_v<T>.hash(),
            std::make_index_sequence<mmclass_storage<T>::field_count()>()
        );
    }

    // ========================================================================-------
    // ======= Binary Serialization
    // ========================================================================-------

    // SFINAE guarantees that non-serializable fields are never serialized
    // 'from' points to start (in memory) of field that we're writing
    template <typename T>
    std::enable_if_t<!is_serializable_v<T>>
    write(const mmfield* self, const void *from, binary_buffer_write& to) {}

    template <typename T>
    std::enable_if_t<is_serializable_v<T>>
    write(const mmfield* self, const void *from, binary_buffer_write& to) { write_serializable<T>(self, from, to); }

    // Serializes primitive types
    template <typename T>
    std::enable_if_t<std::is_fundamental_v<T>>
    write_serializable(const mmfield* self, const void *from, binary_buffer_write& to) {
        to.write(static_cast<const binary_buffer_type*>(from), sizeof(T));
    }

    // Serializes class types
    template <typename C>
    std::enable_if_t<is_hashed_type_v<C>>
    write_serializable(const mmfield* container, const void *from, binary_buffer_write& to) {
        static constexpr hash_type version = classmeta_v<C>.version();
        write<hash_type>(container, &version, to);

        for_each_field<C>([&](const mmfield& field) {
            field.type().actions().Write(&field, field.get_pointer_from(from), to);
        });
    }

    // Serializes std dynamic types
    template <typename D>
    std::enable_if_t<is_vector_v<D> || is_string_v<D>>
    write_serializable(const mmfield* container, const void *from, binary_buffer_write& to) {
        using arr_size_type = typename D::size_type;
        using arr_value_type = typename D::value_type;

        const D* value = static_cast<const D*>(from);
        arr_size_type elementCount = value->size();
        write<arr_size_type>(container, &elementCount, to);
        for(size_t i = 0; i < elementCount; i++) {
            write<arr_value_type>(container, (value->data() + i), to);
        }
    }

    template <typename T>
    std::enable_if_t<is_serializable_v<T>>
    serialize(const T& toSerialize, binary_buffer_write& data) {
        write<T>(nullptr, &toSerialize, data);
    }

    template <typename T>
    std::enable_if_t<!is_serializable_v<T>>
    serialize(const T& toSerialize, binary_buffer_write& data) { }

    // 'from' points to start of field in memory
    // 'to' points to current write location in T
    template <typename T>
    std::enable_if_t<!is_serializable_v<T>>
    read(const mmfield* fieldMeta, binary_buffer_read& from, void *to) {}

    template <typename T>
    std::enable_if_t<is_serializable_v<T>>
    read(const mmfield* fieldMeta, binary_buffer_read& from, void *to) { read_serializable<T>(fieldMeta, from, to); }

    template <typename P>
    std::enable_if_t<std::is_fundamental_v<P>>
    read_serializable(const mmfield* fieldMeta, binary_buffer_read& from, void *to) {
        from.read(static_cast<binary_buffer_type*>(to), sizeof(P));
    }

    template <typename C>
    std::enable_if_t<is_hashed_type_v<C>>
    read_serializable(const mmfield* fieldMeta, binary_buffer_read& from, void *to) {
        hash_type version;
        read<hash_type>(fieldMeta, from, &version);
        
        assert(version == classmeta_v<C>.version() && "Trying to read binary from different version.");

        for_each_field<C>([&](const mmfield& field) {
            field.type().actions().Read(&field, from, field.get_pointer_from(to));
        });
    }

    template <typename D>
    std::enable_if_t<is_vector_v<D> || is_string_v<D>>
    read_serializable(const mmfield* fieldMeta, binary_buffer_read& from, void *to) {
        using arr_size_type = typename D::size_type;
        using arr_value_type = typename D::value_type;
        arr_size_type size = 0;
        read<arr_size_type>(fieldMeta, from, &size);

        D* value = static_cast<D*>(to);
        value->resize(size);
        for(arr_size_type i = 0; i < size; i++) {
            read<arr_value_type>(fieldMeta, from, value->data() + i);
        }
    }

    template <typename T>
    std::enable_if_t<is_serializable_v<T>, T>
    deserialize(binary_buffer_read& buffer) {
        T inst;
        read<T>(nullptr, buffer, (void*) &inst);
        return inst;
    }

    template <typename T>
    std::enable_if_t<!is_serializable_v<T>, T>
    deserialize(binary_buffer_read& buffer) { return T(); }

    // ========================================================================-------
    // ======= YAML Serialization
    // ========================================================================-------

    template <typename T>
    std::enable_if_t<is_serializable_v<T>, YAML::Node>
    serialize_yaml(const T& value) {
        YAML::Node root;
        write_yaml<T>(nullptr, &value, root);
        return root;
    }

    template <typename T>
    std::enable_if_t<is_serializable_v<T>>
    write_yaml(const mmfield* self, const void* from, YAML::Node& parent) { write_serializable_yaml<T>(self, from, parent); }

    template <typename T>
    std::enable_if_t<!is_serializable_v<T>>
    write_yaml(const mmfield* self, const void* from, YAML::Node& parent) {}

    template <typename T>
    std::enable_if_t<std::is_fundamental_v<T> || is_string_v<T>>
    write_serializable_yaml(const mmfield* self, const void* from, YAML::Node& parent) {
        const T* value = static_cast<const T*>(from);
        if(parent.IsSequence())
            parent.push_back(*value);
        else
            parent[self->name().data()] = *value;
    }

    template <typename C>
    std::enable_if_t<is_hashed_type_v<C>>
    write_serializable_yaml(const mmfield* self, const void* from, YAML::Node& parent) {

        YAML::Node objNode = YAML::Node();
        if(parent.IsSequence()) parent.push_back(objNode);
        else if(self) parent[self->name().data()] = objNode;
        else objNode = parent;

        for_each_field<C>([&](const mmfield& field) {
            field.type().actions().WriteYAML(&field, field.get_pointer_from(from), objNode);
        });
    }

    template <typename V>
    std::enable_if_t<is_vector_v<V>>
    write_serializable_yaml(const mmfield* self, const void* from, YAML::Node& parent) {
        using arr_value_type = typename V::value_type;

        YAML::Node seqNode = YAML::Node(YAML::NodeType::value::Sequence);
        if(parent.IsSequence()) parent.push_back(seqNode);
        else parent[self->name().data()] = seqNode;
        
        const V* vecInstance = static_cast<const V*>(from);
        for(const auto& val : *(vecInstance)) {
            write_yaml<arr_value_type>(nullptr, &val, seqNode);
        }
    }

    template <typename T>
    constexpr mmactions type_actions() {
        return { &read<T>, &write<T>, &write_yaml<T> };
    }
}

// ========================================================================-------
// ======= Macro Dark-Magic
// ========================================================================-------
namespace mmeta {
#ifndef __MMETA__
#define MMHASHEDTYPE_DEF(t) \
template <> struct hashed_type<::mmeta::typemeta_v<t>.hash()> { using value_type = t; };

#define MMCLASS_STORAGE(type_name, ...) \
MMHASHEDTYPE_DEF(type_name)\
template <> \
struct mmclass_storage<type_name> { \
    using strg_type = type_name; \
    static constexpr mmfield Fields[]{ __VA_ARGS__ }; \
    static constexpr int field_count() { return sizeof(Fields) / sizeof(mmfield); } \
    static constexpr fieldseq fields() { return { &Fields[0], field_count() }; } \
    static constexpr hash_type version() { return class_version<type_name>(); } \
};

#define MMFIELD_STORAGE(field, ...) { typemeta_v<decltype(strg_type::field)>, #field, offsetof(strg_type, field) }
#else
#define MMHASHEDTYPE_DEF(t)
#define MMCLASS_STORAGE(x, ...)
#define MMFIELD_STORAGE(x, ...)
#endif
}