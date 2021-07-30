#include <mmeta/minimeta.hpp>

// Overrides type_action_traits to enable static polymorphism. This allows
// you to call functions using mmtype as an opaque type.
namespace mmeta {
    template <typename T>
    void print_name() {
        std::cout << "type name is: " << typemeta_v<T>.name() << "\n";
    }

    struct example_actions {
        using PrintNameFn = void (*)();
        const PrintNameFn PrintName;

        constexpr example_actions(const PrintNameFn printNameFn) : PrintName(printNameFn) {}

        template <typename T>
        static constexpr example_actions instantiate() {
            return { &print_name<T> };
        }
    };

    template <typename Meta>
    struct type_action_traits<Meta> {
        using action_type = example_actions;
    };
}

struct SERIALIZABLE Vec3 {
    float X = 0.f, Y = 0.f, Z = 0.f;
};

#include "main.generated.hpp"

int main() {

    mmeta::typemeta_v<Vec3>.actions().PrintName();

    return 0;
}