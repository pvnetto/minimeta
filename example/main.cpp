#include <cstdio>

#include "Components.h"
#include "Examples.h"

namespace mmeta {
    template <typename T>
    constexpr size_t vector_size() {
        if constexpr (is_vector<T>::value) {
            return vector_size<typename T::value_type>();
        }
        return sizeof(T);
    }
}

int main() {
    mmeta::typemeta_v<int>.dump();
    mmeta::typemeta_v<float>.dump();
    mmeta::typemeta_v<Vec3>.dump();

    static_assert(mmeta::typemeta_v<int>.name() == "int" && "Type should be int");
    static_assert(mmeta::is_serializable_v<float> && "Primitive types should all be serializable");
    static_assert(mmeta::is_serializable_v<Vec3> && "User defined types can be serialized");
    static_assert(!mmeta::is_serializable_v<NotSerializable> && "But they don't have to");
    static_assert(mmeta::is_serializable_v<std::vector<int>> && "Even std::vector is serializable though");
    static_assert(!mmeta::is_serializable_v<std::vector<NotSerializable>> && "Unless it's a vector of non-serializables");
    static_assert(mmeta::is_serializable_v<std::vector<std::vector<int>>> && "Nested vectors are also supported");

    // TODO: Go back to using ReadFunc, WriteFunc
    //      - It's probably easier to find a way to define actions than to find a way to map hashes to types automatically,
    //      as templates would add a lot of complexity
    // TODO: Serialize std::string/std::vector (dynamic sized types)
    // TODO: Add serializer versioning
    //      - Check how cista does it
    // TODO: Fix all fixmes

    // Problem: Given a hash, determine if it belongs to a vector/nested vector
    // Solution 1: defined hashed type for vector
    //          - I would have to define all nested levels manually
    // Solution 2: 

    mmeta::binary_buffer dataBuffer;
    {
        Player player;
        player.m_id = 3000;
        player.m_integers = { 0, 1, 2, 3, 4, 5 };
        player.m_nested.push_back({ 10.f, 20.f, 30.f });
        player.m_nested.push_back({ 500.f, 600.f, 700.f });
        player.SetPosition({ 20.f, -20.f, 10.f });
        player.SetName("Paiva");
        
        player.Dump();
        printf("\n");
        mmeta::serialize(player, dataBuffer);
    }

    Player metaPlayer = mmeta::deserialize<Player>(dataBuffer);
    metaPlayer.Dump();

    return 0;
}