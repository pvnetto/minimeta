## Example

Source files parsed by `minimeta` require a compilation database. If you're using CMake, you can't use the Visual Studio generator as it can't generate `compile_commands.json`, which is used to create the compilation database.