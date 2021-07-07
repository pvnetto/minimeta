#include <cstdio>

#include "Components.h"
#include "Examples.h"

#include <vector>
namespace mmeta {
    template <typename T>
    std::enable_if_t<is_serializable_v<T>, binary_buffer>
    serialize(T toSerialize) {
        // Creates data vector and resizes to max possible size
        binary_buffer data;
        data.resize(sizeof(T));

        binary_buffer_type* dataPtr = &data[0];
        auto fields = mmeta::classmeta_v<T>.fields();
        for(const mmfield* field = fields.begin(); field != fields.end(); field++) {
            if(field->type().is_serializable()) {
                memcpy(dataPtr, field->get(&toSerialize), field->type().size());
                dataPtr += field->type().size();
            }
        }

        // Resizes vector to fit exactly all serialized data.
        // FIXME: Optimize this so only one resize is needed
        data.resize(dataPtr - &data[0]);
        return data;
    }

    template <typename T>
    std::enable_if_t<is_serializable_v<T>, T>
    deserialize(const binary_buffer& buffer) {
        T inst;

        const binary_buffer_type* dataPtr = &buffer[0];
        auto fields = mmeta::classmeta_v<T>.fields();
        for(auto field = fields.begin(); field != fields.end(); field++) {
            if(field->type().is_serializable()) {
                field->copy_to(dataPtr, &inst);
                dataPtr += field->type().size();
            }
        }

        return inst;
    }
}

int main() {
    mmeta::typemeta_v<int>.dump();
    mmeta::typemeta_v<float>.dump();
    mmeta::typemeta_v<Vec3>.dump();
    mmeta::classmeta_v<mmeta::teststruct>.dump();

    static_assert(mmeta::typemeta_v<int>.name() == "int" && "Type should be int");
    static_assert(mmeta::is_serializable_v<float> && "Primitive types should all be serializable");
    static_assert(mmeta::is_serializable_v<Vec3> && "User defined types can be serialized");
    static_assert(!mmeta::is_serializable_v<NotSerializable> && "But they don't have to");

    // TODO: Filter which fields are serializable using minimeta tool
    //      - Check how metareflect does it
    //      - Just because a field is serializable doesn't mean it should be serialized, so array filtering wouldn't work
    // TODO: Serialize std::string/std::vector (dynamic sized types)
    // TODO: Fix all fixmes

    mmeta::binary_buffer playerData;
    {
        Player player;
        player.m_id = 3000;
        player.SetPosition({ 20.f, -20.f, 10.f });
        player.SetName("Paiva");
        
        player.Dump();
        printf("\n");
        
        playerData = mmeta::serialize(player);
    }

    Player metaPlayer = mmeta::deserialize<Player>(playerData);
    metaPlayer.Dump();

    return 0;
}