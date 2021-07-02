#include <cstdio>

#include "Components.h"
#include "Examples.h"

constexpr mmeta::mmfield fields[] {
    { mmeta::typemeta_v<int>, "a" },
    { mmeta::typemeta_v<float>, "b" }
};

constexpr mmeta::fieldseq seq { fields, 2 };

int main() {    
    printf("Is Vec3 serializable? %s\n", mmeta::is_serializable<Vec3>::value ? "true" : "false");    
    printf("Is NotSerializable serializable? %s\n", mmeta::is_serializable<NotSerializable>::value ? "true" : "false");

    static constexpr int a[] = { 2, 3, 4 };

    constexpr mmeta::mmfield fieldA { mmeta::typemeta_v<int>, "a" };
    constexpr mmeta::mmclass classMeta { seq };

    static_assert(fieldA.type().hash() == mmeta::typemeta_v<int>.hash(), "Failed.......");
    static_assert(seq.Size == 2, "Failed!!!");
    static_assert(fields[0].name() == "a", "failed....");
    static_assert(mmeta::typemeta_v<int>.name() == "int", "Failed");

    mmeta::typemeta_v<int>.dump();
    mmeta::typemeta_v<float>.dump();
    mmeta::typemeta_v<Vec3>.dump();
    auto classMetadata = mmeta::classmeta_v<mmeta::teststruct>;

    static_assert(mmeta::classmeta_v<mmeta::teststruct>.field_count() == 2, "teststruct has more than 2 fields.");

    return 0;
}