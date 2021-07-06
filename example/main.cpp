#include <cstdio>

#include "Components.h"
#include "Examples.h"

#include <vector>
namespace mmeta {
    template <typename T>
    std::enable_if_t<std::is_arithmetic_v<T>, void>
    process(const T& value) {
        // TODO: Serialize
    }

    template <typename T>
    std::enable_if_t<std::is_class_v<T>, void>
    process(const T& value) {
        // TODO: Serialize
    }

    template <typename T>
    std::enable_if_t<is_serializable_v<T>, void>
    serialize(T&& toSerialize, std::vector<unsigned char>& data) {
        auto classMetadata = mmeta::classmeta_v<T>;
        auto fields = classMetadata.fields();
        for(auto it = fields.begin(); it != fields.end(); it++) {
            // TODO: Serialize field binary data
            // TODO: Get field data from value
        }
    }
}

namespace mmeta {
    template <>
    struct mmclass_storage<Vec3> {
        static constexpr mmfield AllFields[]{
            {typemeta_v<float>, "X", offsetof(Vec3, X)},
            {typemeta_v<float>, "Y", offsetof(Vec3, Y)},
            {typemeta_v<float>, "Z", offsetof(Vec3, Z)},
        };

        static constexpr const mmfield *Fields() { return &AllFields[0]; }
        static constexpr int FieldCount() {
            return sizeof(AllFields) / sizeof(mmfield);
        }
    };
}

int main() {
    printf("Is float serializable? %s\n", mmeta::is_serializable_v<float> ? "true" : "false");
    printf("Is Vec3 serializable? %s\n", mmeta::is_serializable_v<Vec3> ? "true" : "false");
    printf("Is NotSerializable serializable? %s\n", mmeta::is_serializable_v<NotSerializable> ? "true" : "false");


    mmeta::typemeta_v<int>.dump();
    mmeta::typemeta_v<float>.dump();
    mmeta::typemeta_v<Vec3>.dump();
    mmeta::classmeta_v<mmeta::teststruct>.dump();

    static_assert(mmeta::typemeta_v<int>.name() == "int", "Type should be int");
    static_assert(mmeta::classmeta_v<mmeta::teststruct>.field_count() == 3, "teststruct has wrong number of fields.");

    // TODO: Add serializer/deserializer methods
    //      - SOLUTION: Test other serialization tools to see how they work
    //      - Validate which fields are serializable with assert
    // TODO: Filter which fields are serializable using minimeta tool
    //      - Just because a field is serializable doesn't mean it should be serialized, so array filtering wouldn't work
    // TODO: Define serializable fields at compile time from client, instead of running the tool
    // TODO: Serialize std::string/std::vector

    Vec3 vec { 40.f, 0.1f, 0.2f };
    auto vecMeta = mmeta::classmeta_v<Vec3>;
    auto x = vecMeta.fields().begin();
    auto y = x + 1;
    auto z = y + 1;
    
    float metaX = x->get_as<float>(&vec);
    printf("X => %f, X_meta => %f", vec.X, metaX);
    // std::vector<unsigned char> data;
    // mmeta::serialize(vec, data, options);
    // Vec3 outVec = mmeta::deserialize(data, options);

    return 0;
}