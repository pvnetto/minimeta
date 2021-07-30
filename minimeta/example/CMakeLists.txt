option(MMETA_BUILD_COMPONENTS "Build components example along with library" OFF)
option(MMETA_BUILD_STATIC_POLI "Build static polimorphism example along with library" ON)

if(MMETA_BUILD_COMPONENTS)
add_subdirectory(components)
endif()

if(MMETA_BUILD_STATIC_POLI)
add_subdirectory(static_polimorphism)
endif()