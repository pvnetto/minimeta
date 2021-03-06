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
              constexpr std::string_view uglyName = { MMETA_PRETTY_FUNCTION };

              constexpr std::string_view preffix = { MMETA_NAME_PREFFIX }; 
              constexpr size_t preffixIndex = uglyName.find(preffix);
              constexpr std::string_view paddedFront = uglyName.substr(preffixIndex + preffix.size());

              constexpr std::string_view suffix = { MMETA_NAME_SUFFIX }; 
              constexpr size_t suffixIndex = paddedFront.find(suffix);
              return paddedFront.substr(0, suffixIndex);
          }

          static constexpr std::string_view clean_name() {
              constexpr std::string_view namespacedName = prettified_name();
              constexpr const size_t namespaceIdx = namespacedName.find_last_of("::");
              return namespacedName.substr(namespaceIdx + 1);
          }

          static constexpr std::string_view name = clean_name();
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

    using yaml_node = YAML::Node;

    using meta_type = int;

    template <typename Meta>
    class basic_mmfield;
    using mmfield = basic_mmfield<meta_type>;

    struct basic_type_actions {
        using ReadFn = void (*)(const mmfield*, binary_buffer_read&, void *);
        using WriteFn = void (*)(const mmfield*, const void *, binary_buffer_write&);

        using ReadYAMLFn = void (*)(const mmfield*, const yaml_node&, void *);
        using WriteYAMLFn = void (*)(const mmfield*, const void*, yaml_node&);

        constexpr basic_type_actions(const ReadFn readFn, const WriteFn writeFn, const ReadYAMLFn readYamlFn, const WriteYAMLFn writeYamlFn) :
            Read(readFn), Write(writeFn), ReadYAML(readYamlFn), WriteYAML(writeYamlFn) {}

        const ReadFn Read;
        const WriteFn Write;
        const ReadYAMLFn ReadYAML;
        const WriteYAMLFn WriteYAML;

        template <typename T>
        static constexpr basic_type_actions instantiate();
    };

    template <typename Meta, typename = void>
    struct type_action_traits {
        using action_type = basic_type_actions;
    };

    template <typename Meta>
    class basic_mmtype {
    public:
        using action_type = typename type_action_traits<Meta>::action_type;

        constexpr basic_mmtype(const size_t size, const uint64_t hash, std::string_view name, const action_type actions) : 
            m_size(size),
            m_hash(hash),
            m_name(name),
            m_actions(actions) {}

        inline constexpr std::string_view name() const { return m_name; }
        inline constexpr size_t size() const { return m_size; }
        inline constexpr uint64_t hash() const { return m_hash; }
        inline constexpr action_type actions() const { return m_actions; }

        void dump() const {
            std::cout << "type: name => " << name() << ", size => " << size() << ", hash " << hash() << "\n";
        }

    private:
        const size_t m_size;
        const uint64_t m_hash;
        const std::string_view m_name;
        const action_type m_actions;
    };
    using mmtype = basic_mmtype<meta_type>;

    template <typename Meta>
    class basic_mmfield {
    public:
        constexpr basic_mmfield<Meta>(basic_mmtype<Meta> type, std::string_view name, size_t offset) : m_type(type), m_name(name), m_offset(offset) { }

        inline constexpr basic_mmtype<Meta> type() const { return m_type; }
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
            //assert(typemeta_v<meta_type, T>.hash() == m_type.hash() && ">> ERROR: Trying to get field using wrong type.");
            T inst;
            memcpy(&inst, (binary_buffer_type *)src + m_offset, m_type.size());
            return inst;
        }

    private:
        const basic_mmtype<Meta> m_type;
        const std::string_view m_name;
        const size_t m_offset;
    };
    using mmfield = basic_mmfield<meta_type>;

    template <typename Meta>
    struct basic_fieldseq {
        const basic_mmfield<Meta> *Ptr;
        size_t Size;
        
        constexpr basic_fieldseq(const basic_mmfield<Meta>* first, size_t sz) : Ptr(first), Size(sz) {}

        constexpr const basic_mmfield<Meta> *begin() const { return Ptr; }
        constexpr const basic_mmfield<Meta> *end() const { return Ptr + Size; }
        constexpr size_t size() const { return Size; }
    };
    using fieldseq = basic_fieldseq<meta_type>;

    template <typename Meta>
    class basic_mmclass {
    public:
        constexpr basic_mmclass<Meta>(basic_fieldseq<Meta> fields, hash_type version) : m_fields(fields), m_version(version) {}

        constexpr size_t field_count() const { return m_fields.size(); }

        constexpr basic_fieldseq<Meta> fields() const { return m_fields; }

        constexpr hash_type version() const { return m_version; }
        
        void dump() const {
            std::cout  << "class: num_fields => " << field_count() << "\n";
        }
    private:
        const basic_fieldseq<Meta> m_fields;
        const hash_type m_version;
    };
    using mmclass = basic_mmclass<meta_type>;

    template <typename T>
    struct mmclass_storage { };

    
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

    template<typename T, typename Meta = meta_type>
    inline constexpr basic_mmtype<Meta> typemeta_v = {
        sizeof(T),
        utils::hash(utils::type_name<T>::name),
        utils::type_name<T>::name,
        basic_mmtype<Meta>::action_type::template instantiate<T>()};

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

    // ========================================================================-------
    // ======= Class Storage Utils
    // ========================================================================-------

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

    template <typename T, typename Meta = meta_type>
    std::enable_if_t<is_serializable_v<T>>
    serialize(const T& toSerialize, binary_buffer_write& data) {
        write<T>(nullptr, &toSerialize, data);
    }

    template <typename T, typename Meta = meta_type>
    std::enable_if_t<!is_serializable_v<T>>
    serialize(const T& toSerialize, binary_buffer_write& data) { }

    // SFINAE guarantees that non-serializable fields are never serialized
    // 'from' points to start (in memory) of field that we're writing
    template <typename T, typename Meta = meta_type>
    std::enable_if_t<is_serializable_v<T>>
    write(const mmfield* self, const void *from, binary_buffer_write& to) { write_serializable<T>(self, from, to); }

    template <typename T, typename Meta = meta_type>
    std::enable_if_t<!is_serializable_v<T>>
    write(const mmfield* self, const void *from, binary_buffer_write& to) {}

    // Serializes primitive types
    template <typename T, typename Meta = meta_type>
    std::enable_if_t<std::is_fundamental_v<T>>
    write_serializable(const mmfield* self, const void *from, binary_buffer_write& to) {
        to.write(static_cast<const binary_buffer_type*>(from), sizeof(T));
    }

    // Serializes class types
    template <typename C, typename Meta = meta_type>
    std::enable_if_t<is_hashed_type_v<C>>
    write_serializable(const mmfield* container, const void *from, binary_buffer_write& to) {
        static constexpr hash_type version = classmeta_v<C>.version();
        write<hash_type>(container, &version, to);

        for_each_field<C>([&](const mmfield& field) {
            field.type().actions().Write(&field, field.get_pointer_from(from), to);
        });
    }

    // Serializes std dynamic types
    template <typename D, typename Meta = meta_type>
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

    template <typename T, typename Meta = meta_type>
    std::enable_if_t<is_serializable_v<T>, T>
    deserialize(binary_buffer_read& buffer) {
        T inst;
        read<T>(nullptr, buffer, (void*) &inst);
        return inst;
    }

    template <typename T, typename Meta = meta_type>
    std::enable_if_t<!is_serializable_v<T>, T>
    deserialize(binary_buffer_read& buffer) { return T(); }

    // 'from' points to start of field in memory
    // 'to' points to current write location in T
    template <typename T, typename Meta = meta_type>
    std::enable_if_t<!is_serializable_v<T>>
    read(const mmfield* fieldMeta, binary_buffer_read& from, void *to) {}

    template <typename T, typename Meta = meta_type>
    std::enable_if_t<is_serializable_v<T>>
    read(const mmfield* fieldMeta, binary_buffer_read& from, void *to) { read_serializable<T>(fieldMeta, from, to); }

    template <typename P, typename Meta = meta_type>
    std::enable_if_t<std::is_fundamental_v<P>>
    read_serializable(const mmfield* fieldMeta, binary_buffer_read& from, void *to) {
        from.read(static_cast<binary_buffer_type*>(to), sizeof(P));
    }

    template <typename C, typename Meta = meta_type>
    std::enable_if_t<is_hashed_type_v<C>>
    read_serializable(const mmfield* fieldMeta, binary_buffer_read& from, void *to) {
        hash_type version;
        read<hash_type>(fieldMeta, from, &version);
        
        assert(version == classmeta_v<C>.version() && "Trying to read binary from different version.");

        for_each_field<C>([&](const mmfield& field) {
            field.type().actions().Read(&field, from, field.get_pointer_from(to));
        });
    }

    template <typename D, typename Meta = meta_type>
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

    // ========================================================================-------
    // ======= YAML Serialization
    // ========================================================================-------

    template <typename T, typename Meta = meta_type>
    std::enable_if_t<std::is_fundamental_v<T> || is_string_v<T>>
    write_serializable_yaml(const basic_mmfield<Meta>* self, const void* from, yaml_node& to) {
        const T* value = static_cast<const T*>(from);
        to = *value;
    }

    template <typename C, typename Meta = meta_type>
    std::enable_if_t<is_hashed_type_v<C>>
    write_serializable_yaml(const basic_mmfield<Meta>* self, const void* from, yaml_node& to) {
        for_each_field<C>([&](const basic_mmfield<Meta>& field) {
            yaml_node fieldNode = to[field.name().data()];
            field.type().actions().WriteYAML(&field, field.get_pointer_from(from), fieldNode);
        });
    }

    template <typename V, typename Meta = meta_type>
    std::enable_if_t<is_vector_v<V>>
    write_serializable_yaml(const basic_mmfield<Meta>* self, const void* from, yaml_node& to) {
        using arr_value_type = typename V::value_type;

        const V* vecInstance = static_cast<const V*>(from);
        for(const auto& val : *(vecInstance)) {
            yaml_node seqNode = yaml_node();
            to.push_back(seqNode);
            write_yaml<arr_value_type, Meta>(nullptr, &val, seqNode);
        }
    }

    template <typename T, typename Meta = meta_type>
    std::enable_if_t<is_serializable_v<T>>
    write_yaml(const basic_mmfield<Meta>* self, const void* from, yaml_node& to) { write_serializable_yaml<T>(self, from, to); }

    template <typename T, typename Meta = meta_type>
    std::enable_if_t<!is_serializable_v<T>>
    write_yaml(const basic_mmfield<Meta>* self, const void* from, yaml_node& to) {}

    template <typename T, typename Meta = meta_type>
    std::enable_if_t<is_serializable_v<T>>
    serialize_yaml(const T& value, yaml_node& root) {
        write_yaml<T, Meta>(nullptr, &value, root);
    }

    template <typename T, typename Meta = meta_type>
    std::enable_if_t<is_serializable_v<T>, T>
    deserialize_yaml(const yaml_node& from) {
        T inst;
        read_yaml<T, Meta>(nullptr, from, &inst);
        return inst;
    }

    template <typename T, typename Meta = meta_type>
    std::enable_if_t<!is_serializable_v<T>, T>
    deserialize_yaml(const yaml_node& from) { return T(); }

    template <typename T, typename Meta = meta_type>
    std::enable_if_t<!is_serializable_v<T>>
    read_yaml(const basic_mmfield<Meta>* self, const yaml_node& from, void *to) {}

    template <typename T, typename Meta = meta_type>
    std::enable_if_t<is_serializable_v<T>>
    read_yaml(const basic_mmfield<Meta>* self, const yaml_node& from, void *to) { read_serializable_yaml<T>(self, from, to); }

    template <typename T, typename Meta = meta_type>
    std::enable_if_t<std::is_fundamental_v<T> || is_string_v<T>>
    read_serializable_yaml(const basic_mmfield<Meta>* self, const yaml_node& from, void *to) {
        T* valuePtr = static_cast<T*>(to);
        *valuePtr= from.as<T>();
    }

    template <typename C, typename Meta = meta_type>
    std::enable_if_t<is_hashed_type_v<C>>
    read_serializable_yaml(const basic_mmfield<Meta>* self, const yaml_node& from, void *to) {
        for_each_field<C>([&](const basic_mmfield<Meta>& field) {
            yaml_node yamlField = from[field.name().data()];
            field.type().actions().ReadYAML(&field, yamlField, field.get_pointer_from(to));
        });
    }

    template <typename V, typename Meta = meta_type>
    std::enable_if_t<is_vector_v<V>>
    read_serializable_yaml(const basic_mmfield<Meta>* self, const yaml_node& from, void *to) {
        using arr_size_type = typename V::size_type;
        using arr_value_type = typename V::value_type;

        V* value = static_cast<V*>(to);
        value->resize(from.size());

        arr_size_type i = 0;
        for(auto& yamlField : from) {
            read_yaml<arr_value_type>(self, yamlField, value->data() + i);
            i++;
        }
    }

    template <typename T>
    constexpr basic_type_actions basic_type_actions::instantiate() {
        return {
            &read<T>, &write<T>, &read_yaml<T>, &write_yaml<T>
        };
    };
}

// ========================================================================-------
// ======= Macro Dark-Magic
// ========================================================================-------
namespace mmeta {
#ifndef __MMETA__
#define MMETA_CLASS_STORAGE(type_name, ...) \
template <> \
struct mmclass_storage<type_name> { \
    using strg_type = type_name; \
    static constexpr mmfield Fields[]{ __VA_ARGS__ }; \
    static constexpr int field_count() { return sizeof(Fields) / sizeof(mmfield); } \
    static constexpr fieldseq fields() { return { &Fields[0], field_count() }; } \
    static constexpr hash_type version() { return class_version<type_name>(); } \
};

#define MMETA_FIELD(field, ...) { typemeta_v<decltype(strg_type::field)>, #field, offsetof(strg_type, field) }

#define MMETA_CLASS(type_name, ...) \
    namespace mmeta { \
        template <> \
        struct is_serializable<type_name> : std::true_type {}; \
        template <> \
        struct hashed_type<::mmeta::typemeta_v<type_name>.hash()> { using value_type = type_name; }; \
        MMETA_CLASS_STORAGE(type_name, __VA_ARGS__) \
    }
#else
#define MMETA_CLASS_STORAGE(x, ...)
#define MMETA_FIELD(x, ...)
#define MMETA_CLASS(type_name, ...)
#endif

}