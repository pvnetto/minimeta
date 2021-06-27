#include <cstdio>

#include "Components.h"
#include "Examples.h"

int main() {
    printf("Is Vec3 serializable? %s\n", mmeta::is_serializable<Vec3>::value ? "true" : "false");    
    printf("Is NotSerializable serializable? %s\n", mmeta::is_serializable<NotSerializable>::value ? "true" : "false");    
    return 0;
}