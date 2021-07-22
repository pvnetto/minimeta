# Minimeta

Minimeta is a header-only, small and easy-to-use reflection library that generates **compile-time C++ type metadata**, while also containing **built-in serialization** utilities. The main difference between minimeta and other libraries is that it **comes with a Clang Libtooling utility** that can be used in a pre-build step to **automatically generate reflection code**, unlike most of the other tools around, where you have to manually type in which fields you want it to generate. It's written in C++17 and makes heavy use of template metaprogramming and constexpr shennanigans.

### Motivation

The reason I wrote Minimeta is because pretty much every reflection library for C++ I've seen either has horrendous syntax or require a lot of manual typing, sometimes even both. Having to manually type things like versions, which fields to serialize etc is prone to human errors and bound to introduce bugs, so I've decided to make a library that solves both those problems, but still gives the user the option to do it manually, if he wants to.

### Use Cases

Serialization between machines with different pointer width/endianness is currently **not supported**. For all else this should be fine.

Also, the supported data types are:

- Fundamental types
- User-defined PODs
- std::vector
- std::string

But it's trivial to add support to other types.

## Getting Started

Minimeta requires C++17 or later and was tested with all major compilers (MSVC, GCC and Clang). I've only run it on Windows though, so it's not guaranteed to work on other platforms.

There are two different ways to use Minimeta: With or without the LibTooling pre-build step.

### No Libtooling

Minimeta is a header-only library, so all you have to do is include the headers in your build configuration. Usually, you can do something along those lines:

```batch
cd path/to/your/application
git clone https://github.com/pvnetto/minimeta --recursive
```

```cmake
# Inside your project's CMakeLists.txt
add_subdirectory(path/to/minimeta/minimeta)
target_link_libraries(your-target minimeta)
target_include_directories(your-target
    PUBLIC ${PROJECT_SOURCE_DIR}/path/to/minimeta/include)
```

### With Libtooling

Minimeta comes with a tool made with [LLVM/Clang LibTooling](https://clang.llvm.org/docs/LibTooling.html) that parses your source code and creates reflection data for your annotated types (check the [Code Examples](https://github.com/pvnetto/minimeta#code-examples)).

In order to make use of this tool, you should first **follow the [No LibTooling steps](https://github.com/pvnetto/minimeta#no-libtooling)** and then **install** LLVM along with the tool:

0) [Build/Install LLVM](https://llvm.org/docs/CMake.html).
	- **This should take a few hours**.
	
1) Clone this project to `your/path/to/llvm/clang-tools-extra`

```bat
cd path/to/llvm/clang-tools-extra
git clone https://github.com/pvnetto/minimeta
```

2) Open `your/path/to/llvm/clang-tools-extra/CMakeLists.txt` and add this line:

```cmake
add_subdirectory(minimeta/libtooling)
```

3) Rebuild LLVM from inside `your/path/to/llvm/your-build-folder`

```bat
cd your/path/to/llvm/build-folder
cmake --build .
```

If you've done everything correctly, the tool should be available from your command line. 

Source files parsed by LibTooling require a compilation database. Here's a nice source on [what is a compilation database and how to generate it](https://www.jetbrains.com/help/clion/compilation-database.html) for your project. If you're using CMake, **you can't use the Visual Studio generator** as it can't generate `compile_commands.json`.

Now that you installed the tool and generated a compilation database for your file, you should run minimeta from CLI:

```batch
cd path/to/your/project/root
minimeta yourentrypoint.cpp -p path/to/your/compile_commands.json --extra-arg "/std:c++17"
```

This will generate a source file for each of your files that contains annotations. Beware that **you don't have to run this from your entry point** file, you could also generate source files from other specific files, but running it from your entry point makes sure all files are parsed.

You can also check out examples on how to use the LibTooling approach [here](https://github.com/pvnetto/minimeta/tree/master/minimeta/example).

## Code Examples

### Minimeta LibTooling

Minimeta Libtooling automatically generates code for compile-time reflection, and all you have to do is annotate your classes. It uses very simple rules to determine what to reflect:

- All classes with `SERIALIZABLE` are **reflected**;
- All **public** fields are **reflected**, unless annotated with `INTERNAL`;
- All **private** fields are **ignored**, unless annotated with `SERIALIZE`;
- If you're reflecting a private field, annotate your class with `META_OBJECT`;

If you're used to Unity3D, those rules will sound very familiar.


```cpp
#include <mmeta/annotations.h>

struct SERIALIZABLE Vec3 {
	float X, Y, Z;
};

class SERIALIZABLE Player {
public:
	Vec3 m_position;
	float INTERNAL m_points;
private:
	std::string SERIALIZE m_name;

	META_OBJECT
};
```

### No Minimeta LibTooling

Minimeta makes use of macros to reflect data under the hood. When you're using LibTooling you don't need to type those macros, as the code generator will write them for your, but in this case you would have to. This is what it looks like:

```cpp
#include <mmeta/minimeta.hpp>

struct Vec3 {
	float X, Y, Z;
};

MMETA_CLASS(
	Vec3,
	MMETA_FIELD(X),
	MMETA_FIELD(Y),
	MMETA_FIELD(Z))

class Player {
public:
	Vec3 m_position;
	float m_points;
private:
	std::string m_name;

	META_OBJECT
};

MMETA_CLASS(
	Player,
	MMETA_FIELD(m_position),
	MMETA_FIELD(m_name))
```

## Dependencies

- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
