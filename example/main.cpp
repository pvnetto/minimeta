#if defined(__clang__) || defined(__GNUC__)
#define SERIALIZABLE __attribute__((annotate("mm-type")))
#define SERIALIZE __attribute__((annotate("mm-add")))
#define INTERNAL __attribute__((annotate("mm-ignore")))
#else
#define SERIALIZABLE
#define SERIALIZE
#define INTERNAL
#endif

int main() {
    return 0;
}

struct SERIALIZABLE Vec3 {
    float X, Y, Z;
};

class SERIALIZABLE Player {
public:
    Vec3 m_position;

private:
    const char* name = "Paiva";
};

struct NotSerializable {
    float a, b, c;
};

class SERIALIZABLE CustomAccessors {
public:
    float show;
    float INTERNAL dontShow;

private:
    float SERIALIZE showPrivate;
};

struct SERIALIZABLE MoreCustomAccessors {
    float Show;
    float INTERNAL DontShow;
};
