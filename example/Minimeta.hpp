#pragma once

#include <iostream>

#include <stdint.h>
#include <type_traits>
#include <typeinfo>
#include <cassert>

#include <string_view>
#include <vector>


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

#define BIT(x) (1 << x)

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
    using binary_buffer_type = uint8_t;
    using binary_buffer = std::vector<binary_buffer_type>;

    // ========================================================================-------
    // ======= Types
    // ========================================================================-------
    class mmfield;

    using WriteFunc = void(*)(const mmfield*, const void *, binary_buffer_type*&);
    using ReadFunc = void(*)(const mmfield*, const binary_buffer_type*&, void *);

    class mmtype {
    public:
        constexpr mmtype(const uint64_t size, const uint64_t hash, std::string_view name, bool serializable, const ReadFunc readFn, const WriteFunc writeFn) : 
            m_size(size),
            m_hash(hash),
            m_name(name),
            m_serializable(serializable),
            m_read(readFn),
            m_write(writeFn) { }

        inline constexpr std::string_view name() const { return m_name; }
        inline constexpr uint64_t size() const { return m_size; }
        inline constexpr uint64_t hash() const { return m_hash; }
        inline constexpr bool is_serializable() const { return m_serializable; }
        inline void write(const mmfield* ref, const void *src, binary_buffer_type*& dst) const { m_write(ref, src, dst); }
        inline void read(const mmfield* field, const binary_buffer_type*& from, void *to) const { m_read(field, from, to); }

        void dump() const {
            std::cout << "type: name => " << name() << ", size => " << size() << ", hash " << hash() << "\n";
        }

    private:
        const uint64_t m_size;
        const uint64_t m_hash;
        const std::string_view m_name;
        const bool m_serializable;
        const ReadFunc m_read;
        const WriteFunc m_write;
    };

    class mmfield {
    public:
        constexpr mmfield(mmtype type, std::string_view name, size_t offset) : m_type(type), m_name(name), m_offset(offset) { }

        inline constexpr mmtype type() const { return m_type; }
        inline constexpr std::string_view name() const { return m_name; }

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
        constexpr mmclass(fieldseq fields) : m_fields(fields) { }

        constexpr size_t field_count() const { return m_fields.size(); }

        constexpr fieldseq fields() const { return m_fields; }
        
        void dump() const {
            std::cout  << "class: num_fields => " << field_count() << "\n";
        }
    private:
        const fieldseq m_fields;
    };

    template <typename T>
    struct mmclass_storage {
        static constexpr mmfield* Fields() { return nullptr; }
        static constexpr int FieldCount() { return 0; }
    };

    
    // ========================================================================-------
    // ======= Type traits
    // ========================================================================-------
    template <typename T>
    using class_type = std::enable_if_t<std::is_class_v<T>, mmclass>;

    template<class T>
    struct is_vector : std::false_type { };

    template<typename T>
    struct is_vector<std::vector<T>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_vector_v = is_vector<T>::value;

    template<class T>
    struct is_string : std::false_type { };

    template<>
    struct is_string<std::string> : std::true_type {};

    template<typename T>
    inline constexpr bool is_string_v = is_string<T>::value;

    template <typename, class = void>
    struct is_defined : std::false_type {};
    
    template <typename T>
    struct is_defined<T,
        std::enable_if_t<std::is_object<T>::value &&
                        !std::is_pointer<T>::value &&
                        (sizeof(T) > 0)
        >
    > : std::true_type { };

#ifdef __MMETA__
    template <typename T>
    struct is_serializable : std::true_type {};
#else
    template <typename T>
    struct is_serializable {
        static_assert(is_defined<T>::value && "is_serializable isn't really useful with forward declared types.");
        static constexpr bool value = std::is_fundamental_v<T>;
    };

    // template <typename T>
    // struct is_serializable<std::vector<T>> : std::true_type {
    //     template <typename B>
    //     static constexpr bool is_vector_of_serializable() {
    //         if constexpr (is_vector<B>::value) {
    //             return is_vector_of_serializable<typename B::value_type>();
    //         }
    //         return is_serializable_v<B>;
    //     }

    //     static constexpr bool value = is_vector_of_serializable<T>();
    // };
#endif

    template<typename T>
    inline constexpr bool is_serializable_v = is_serializable<T>::value;


    // ========================================================================-------
    // ======= Serialization
    // ========================================================================-------

    // Example:
    // Foo
    //      float c
    //      char d
    //      Bar a
    //          int a, int b

    // write(Foo)
    //      write(c) => Gets c from Foo, writes to memory
    //      write(d) => Gets d from Foo, writes to memory
    //      write(Bar) => Gets Bar from Foo (src)
    //              write(a) => Gets a from Bar (src), writes to memory (dst)
    //              write(b) => Gets b from bar (src), writes to memory

    // SFINAE guarantees that non-serializable fields are never serialized
    // 'src' points to start of data that is being written
    // 'to' points to current write location in buffer 
    template <typename T>
    std::enable_if_t<!is_serializable_v<T>, void>
    write(const mmfield* self, const void *src, binary_buffer_type*& to) { }

    template <typename T>
    std::enable_if_t<is_serializable_v<T>, void>
    write(const mmfield* self, const void *src, binary_buffer_type*& to) { write_serializable<T>(self, src, to); }

    template <typename T>
    std::enable_if_t<std::is_fundamental_v<T>, void>
    write_serializable(const mmfield* self, const void *primitiveBeginPtr, binary_buffer_type*& to) {
        const size_t fieldSize = self->type().size();
        memcpy(to, primitiveBeginPtr, fieldSize);
        to += fieldSize;
    }

    template <typename T>
    std::enable_if_t<std::is_class_v<T>, void>
    write_serializable(const mmfield* self, const void *classBeginPtr, binary_buffer_type*& to) {
        // Serializes all child fields, SFINAE will guarantee that non-serializable fields are not included
        auto fieldsToWrite = mmeta::classmeta_v<T>.fields();
        for(const mmfield* fieldToWrite = fieldsToWrite.begin(); fieldToWrite != fieldsToWrite.end(); fieldToWrite++) {
            // Gets pointer to start of child field and writes it
            fieldToWrite->type().write(fieldToWrite, fieldToWrite->get_pointer_from(classBeginPtr), to);
        }
    }

    template <typename T>
    std::enable_if_t<is_vector_v<T>, void>
    write_serializable(const mmfield* parentField, const void *src, binary_buffer_type*& dst) { }

    template <typename T>
    std::enable_if_t<is_serializable_v<T>, binary_buffer>
    serialize(T toSerialize) {
        // Creates data vector and resizes to max possible size
        binary_buffer data;
        data.resize(sizeof(T));

        binary_buffer_type* dst = &data[0];
        mmeta::typemeta_v<T>.write(nullptr, &toSerialize, dst);

        // Resizes vector to fit exactly all serialized data.
        data.resize(dst - &data[0]);
        return data;
    }


    // Skips non-serializable data
    // 'from' points to start of field in memory
    // 'to' points to current write location in T
    template <typename T>
    std::enable_if_t<!is_serializable_v<T>, void>
    read(const mmfield* fieldMeta, const binary_buffer_type*& from, void *to) {}

    template <typename T>
    std::enable_if_t<is_serializable_v<T>, void>
    read(const mmfield* fieldMeta, const binary_buffer_type*& from, void *to) { read_serializable<T>(fieldMeta, from, to); }

    template <typename T>
    std::enable_if_t<std::is_fundamental_v<T>, void>
    read_serializable(const mmfield* fieldMeta, const binary_buffer_type*& from, void *to) {
        const size_t fieldSize = fieldMeta->type().size();
        memcpy(to, from, fieldSize);
        from += fieldSize;
    }

    template <typename T>
    std::enable_if_t<std::is_class_v<T>, void>
    read_serializable(const mmfield* fieldMeta, const binary_buffer_type*& from, void *to) {
        auto fieldsToRead = mmeta::classmeta_v<T>.fields();
        for(const mmfield* fieldToRead = fieldsToRead.begin(); fieldToRead != fieldsToRead.end(); fieldToRead++) {
            // TODO: Set write location to start of field in target's data layout
            fieldToRead->type().read(fieldToRead, from, fieldToRead->get_pointer_from(to));
        }
    }

    template <typename T>
    std::enable_if_t<is_serializable_v<T>, T>
    deserialize(const binary_buffer& buffer) {
        T inst;

        const binary_buffer_type* from = &buffer[0];
        mmeta::typemeta_v<T>.read(nullptr, from, (void*) &inst);

        return inst;
    }

    // ========================================================================-------
    // ======= Metadata Generators
    // ========================================================================-------
    
    template<typename T>
    struct typemeta {
        static constexpr mmtype value = {
            sizeof(T),
            utils::hash(utils::type_name<T>::name),
            utils::type_name<T>::name,
            is_serializable_v<T>,
            &read<T>,
            &write<T>};
    };

    template<typename T>
    inline constexpr mmtype typemeta_v = typemeta<T>::value;

    template<typename T>
    struct classmeta {
        static constexpr mmclass value = {
            { mmclass_storage<T>::Fields(), mmclass_storage<T>::FieldCount()  }
        };
    };

    template<typename T>
    inline constexpr class_type<T> classmeta_v = classmeta<T>::value;

    // ========================================================================-------
    // ======= Macro Dark-Magic
    // ========================================================================-------

#ifndef __MMETA__
#define MMCLASS_STORAGE(type_name, ...) \
template <> \
struct mmclass_storage<type_name> { \
    using strg_type = type_name; \
    static constexpr mmfield AllFields[]{ __VA_ARGS__ }; \
\
    static constexpr const mmfield *Fields() { return &AllFields[0]; } \
    static constexpr int FieldCount() { \
        return sizeof(AllFields) / sizeof(mmfield); \
    } \
};

#define MMFIELD_STORAGE(field, ...) { typemeta_v<decltype(strg_type::##field)>, #field, offsetof(strg_type, field) }
#else
#define MMCLASS_STORAGE(x, ...)
#define MMFIELD_STORAGE(x, ...)
#endif

    struct notserializable {
        int dkajshda;
    };

    struct teststruct {
        int A;
        char B;
        notserializable Not;
    };

    MMCLASS_STORAGE(
        teststruct,
        MMFIELD_STORAGE(A),
        MMFIELD_STORAGE(B),
        MMFIELD_STORAGE(Not)
    )
}