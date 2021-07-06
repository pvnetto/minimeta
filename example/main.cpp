#include <cstdio>

#include "Components.h"
#include "Examples.h"

#include <vector>
namespace mmeta {
    template <typename T>
    std::enable_if_t<is_serializable_v<T>, std::vector<char>>
    serialize(T toSerialize) {
        // Creates data vector and resizes to max possible size
        std::vector<char> dataBuffer;
        dataBuffer.resize(sizeof(T));

        char* dataPtr = &dataBuffer[0];
        auto fields = mmeta::classmeta_v<T>.fields();
        for(const mmfield* field = fields.begin(); field != fields.end(); field++) {
            // TODO: Assert if field is serializable
            memcpy(dataPtr, field->get(&toSerialize), field->type().size());
            dataPtr += field->type().size();
        }

        // Resizes vector to fit exactly all serialized data.
        // FIXME: Optimize this so only one resize is needed
        dataBuffer.resize(dataPtr - &dataBuffer[0]);
        return dataBuffer;
    }

    template <typename T>
    std::enable_if_t<is_serializable_v<T>, T>
    deserialize(char * dataBegin) {
        T inst;

        char* dataPtr = dataBegin;
        auto fields = mmeta::classmeta_v<T>.fields();
        for(auto it = fields.begin(); it != fields.end(); it++) {
            it->copy_to(dataPtr, &inst);
            dataPtr += it->type().size();
        }

        return inst;
    }
}

namespace mmeta {
    MMCLASS_STORAGE(
        Vec3,
        MMFIELD_STORAGE(Vec3, X),
        MMFIELD_STORAGE(Vec3, Y),
        MMFIELD_STORAGE(Vec3, Z)       
    )
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

    // TODO: Filter which fields are serializable using minimeta tool
    //      - Just because a field is serializable doesn't mean it should be serialized, so array filtering wouldn't work
    // TODO: Define serializable fields at compile time from client, instead of running the tool
    // TODO: Serialize std::string/std::vector (dynamic sized types)

    std::vector<char> data;
    {
        Vec3 vec { 4.f, 0.1f, 0.2f };
        data = mmeta::serialize(vec/*, mode, options*/);
    }

    // auto x = mmeta::classmeta_v<Vec3>.fields().begin();
    // auto y = x + 1;
    // auto z = x + 2;

    // float metaX = x->get_as<float>(&data[0]);
    // float metaY = y->get_as<float>(&data[0]);
    // float metaZ = z->get_as<float>(&data[0]);

    // printf("X => %f\n", metaX);
    // printf("Y => %f\n", metaY);
    // printf("Z => %f\n\n", metaZ);

    Vec3 metaVec = mmeta::deserialize<Vec3>(&data[0]/*, mode, options*/);
    printf("X => %f\n", metaVec.X);
    printf("Y => %f\n", metaVec.Y);
    printf("Z => %f\n", metaVec.Z);

    return 0;
}