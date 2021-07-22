#include "Components.h"
#include "Examples.h"

#include <mmeta/minimeta.hpp>

int main() {
    // compile-time type metadata
    mmeta::typemeta_v<int>.dump();
    mmeta::typemeta_v<float>.dump();
    mmeta::typemeta_v<Vec3>.dump();

    // compile-time assertions over types
    static_assert(mmeta::typemeta_v<int>.name() == "int" && "Type name should be int");
    static_assert(mmeta::typemeta_v<int>.hash() != 0 && "Every type has a unique hash identifier generated at compile-time");
    static_assert(mmeta::is_serializable_v<float> && "Primitive types are all serializable");
    static_assert(mmeta::is_serializable_v<Vec3> && "User defined types can be serialized, as long as they're annotated");
    static_assert(!mmeta::is_serializable_v<NotSerializable> && "But they don't have to");
    static_assert(mmeta::is_serializable_v<std::vector<int>> && "std::vector should be serializable");
    static_assert(!mmeta::is_serializable_v<std::vector<NotSerializable>> && "std::vector of non-serializables can't be serialized");
    static_assert(mmeta::is_serializable_v<std::vector<std::vector<int>>> && "Nested std::vectors must be serializable");
    static_assert(mmeta::is_serializable_v<std::string> && "std::string must be serializable");
    static_assert(mmeta::classmeta_v<Vec3>.version() != mmeta::classmeta_v<ColorRGB>.version() && "Binary serializer is versioned using hashes.");

    // binary/yaml serialization
    mmeta::binary_buffer dataBuffer;
    mmeta::yaml_node dataNode;
    {
        Player player;
        player.m_id = 3000;
        player.m_integers = { 0, 1, 2, 3, 4, 5 };
        player.m_nested.push_back({ 10.f, 20.f, 30.f });
        player.m_nested.push_back({ 500.f, 600.f, 700.f });
        player.SetPosition({ 20.f, -20.f, 10.f });
        player.SetName("Paiva");
        
        player.m_targets.push_back({ -30.f, -30.f, 0.f });
        player.m_targets.push_back({ -50.f, 0.f, 50.f });

        player.Dump();

        mmeta::serialize(player, dataBuffer);
        mmeta::serialize_yaml(player, dataNode);
    }

    Player metaPlayer = mmeta::deserialize<Player>(dataBuffer);
    metaPlayer.Dump();

    Player yamlPlayer = mmeta::deserialize_yaml<Player>(dataNode);
    yamlPlayer.Dump();

    //YAML::Emitter emitter;
    //emitter << dataNode;
    //std::cout << emitter.c_str() << "\n";

    return 0;
}