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

// FIXME: Add preffix/suffix for clang and gcc
// FIXME: Test if it works on clang/gcc
// source: https://github.com/Manu343726/ctti/blob/master/include/ctti/detail/pretty_function.hpp
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

        constexpr mmactions(const ReadFn readFn, const WriteFn writeFn) :
            Read(readFn), Write(writeFn) { }

        const ReadFn Read;
        const WriteFn Write;
    };

    class mmtype {
    public:
        constexpr mmtype(const size_t size, const uint64_t hash, std::string_view name, const mmactions actions) : 
            m_size(size),
            m_hash(hash),
            m_name(name),
            m_actions(actions) { }

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
        static constexpr bool value = is_serializable_v<T>;
    };

    template <>
    struct is_serializable<std::string> : std::true_type {};
#endif

    template<typename T>
    inline constexpr bool is_serializable_v = is_serializable<T>::value;

    // FIXME: Move to some kind of utility part of the code
    template<typename T, template<typename T, int> class W, std::size_t... I, typename... Ts>
    void each_field_impl(std::index_sequence<I...>, Ts&... args) {
        int t[] = { 0, ((void)W<T, I>()(args...), 1)... };
        (void) t;
    }  

    template<typename T, template<typename T, int> class W, typename... Ts>
    void each_field(Ts&... args) {
        static constexpr std::size_t n = mmeta::classmeta_v<T>.fields().size();
        each_field_impl<T, W>(std::make_index_sequence<n>(), args...);
    }

    template <typename T, std::size_t... I>
    static constexpr std::enable_if_t<is_hashed_type_v<T>, hash_type>
    combine_hashes(hash_type h, std::index_sequence<I...>) {
        auto mix = [&](std::string_view other) {
            h = utils::hash(other.data(), h);
        };
        ((mix(mmclass_storage<T>::AllFields[I].type().name())), ...);
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
    // ======= Serialization
    // ========================================================================-------

    // SFINAE guarantees that non-serializable fields are never serialized
    // 'from' points to start (in memory) of field that we're writing
    template <typename T>
    std::enable_if_t<!is_serializable_v<T>, void>
    write(const mmfield* self, const void *from, binary_buffer_write& to) {}

    template <typename T>
    std::enable_if_t<is_serializable_v<T>, void>
    write(const mmfield* self, const void *from, binary_buffer_write& to) { write_serializable<T>(self, from, to); }

    // Serializes primitive types
    template <typename T>
    std::enable_if_t<std::is_fundamental_v<T>, void>
    write_serializable(const mmfield* self, const void *from, binary_buffer_write& to) {
        const size_t fieldSize = self->type().size();
        to.write(static_cast<const binary_buffer_type*>(from), sizeof(T));
    }

    template<typename T, int I>
    struct field_write_wrapper {
        // 'from' points to start of field's container. This is useful to get field's location in memory layout
        void operator()(const mmfield* container, const void *from, binary_buffer_write& to) const {
            static constexpr const mmeta::mmfield field = mmeta::mmclass_storage<T>::AllFields[I];
            field.type().actions().Write(&field, field.get_pointer_from(from), to);
        }
    };

    // Serializes class types
    template <typename C>
    std::enable_if_t<is_hashed_type_v<C>, void>
    write_serializable(const mmfield* container, const void *from, binary_buffer_write& to) {
        each_field<C, field_write_wrapper>(container, from, to);
    }

    // Serializes std dynamic types
    template <typename D>
    std::enable_if_t<is_vector_v<D> || is_string_v<D>, void>
    write_serializable(const mmfield* container, const void *from, binary_buffer_write& to) {
        using arr_size_type = typename D::size_type;
        using arr_value_type = typename D::value_type;

        const D* value = reinterpret_cast<const D*>(from);
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
    std::enable_if_t<!is_serializable_v<T>, void>
    read(const mmfield* fieldMeta, binary_buffer_read& from, void *to) {}

    template <typename T>
    std::enable_if_t<is_serializable_v<T>, void>
    read(const mmfield* fieldMeta, binary_buffer_read& from, void *to) { read_serializable<T>(fieldMeta, from, to); }

    template <typename P>
    std::enable_if_t<std::is_fundamental_v<P>, void>
    read_serializable(const mmfield* fieldMeta, binary_buffer_read& from, void *to) {
        from.read(static_cast<binary_buffer_type*>(to), sizeof(P));
    }

    template<typename T, int i>
    struct read_field_wrapper {
        void operator()(const mmfield* fieldMeta, binary_buffer_read& from, void *to) const {
            // 'to' points to start (in memory) of class that contains field. This is useful to
            // get field's location in the container's memory layout.
            static constexpr const mmeta::mmfield field = mmeta::mmclass_storage<T>::AllFields[i];
            field.type().actions().Read(&field, from, field.get_pointer_from(to));
        }
    };

    template <typename C>
    std::enable_if_t<is_hashed_type_v<C>, void>
    read_serializable(const mmfield* fieldMeta, binary_buffer_read& from, void *to) {
        each_field<C, read_field_wrapper>(fieldMeta, from, to);
    }

    template <typename D>
    std::enable_if_t<is_vector_v<D> || is_string_v<D>, void>
    read_serializable(const mmfield* fieldMeta, binary_buffer_read& from, void *to) {
        using arr_size_type = typename D::size_type;
        using arr_value_type = typename D::value_type;
        arr_size_type size = 0;
        read<arr_size_type>(fieldMeta, from, &size);

        D* value = reinterpret_cast<D*>(to);
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

    template <typename T>
    constexpr mmactions type_actions() {
        return { &read<T>, write<T> };
    }

    // ========================================================================-------
    // ======= Macro Dark-Magic
    // ========================================================================-------
#ifndef __MMETA__
#define MMHASHEDTYPE_DEF(t) \
template <> struct hashed_type<::mmeta::typemeta_v<t>.hash()> { using value_type = t; };

#define MMCLASS_STORAGE(type_name, ...) \
MMHASHEDTYPE_DEF(type_name)\
template <> \
struct mmclass_storage<type_name> { \
    using strg_type = type_name; \
    static constexpr mmfield AllFields[]{ __VA_ARGS__ }; \
    static constexpr int field_count() { return sizeof(AllFields) / sizeof(mmfield); } \
    static constexpr fieldseq fields() { return { &AllFields[0], field_count() }; } \
    static constexpr hash_type version() { return class_version<type_name>(); } \
};

#define MMFIELD_STORAGE(field, ...) { typemeta_v<decltype(strg_type::##field)>, #field, offsetof(strg_type, field) }
#else
#define MMHASHEDTYPE_DEF(t)
#define MMCLASS_STORAGE(x, ...)
#define MMFIELD_STORAGE(x, ...)
#endif
}