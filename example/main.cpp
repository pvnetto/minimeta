#include <cstdio>

#include "Components.h"
#include "Examples.h"

int main() {
    printf("Is float serializable? %s\n", mmeta::is_serializable_v<float> ? "true" : "false");
    printf("Is Vec3 serializable? %s\n", mmeta::is_serializable_v<Vec3> ? "true" : "false");
    printf("Is NotSerializable serializable? %s\n", mmeta::is_serializable_v<NotSerializable> ? "true" : "false");


    mmeta::typemeta_v<int>.dump();
    mmeta::typemeta_v<float>.dump();
    mmeta::typemeta_v<Vec3>.dump();
    mmeta::classmeta_v<mmeta::teststruct>.dump();

    static_assert(mmeta::typemeta_v<int>.name() == "int", "Failed");
    static_assert(mmeta::classmeta_v<mmeta::teststruct>.field_count() == 3, "teststruct has more than 2 fields.");

    return 0;
}