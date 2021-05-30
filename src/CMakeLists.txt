# INTERFACE specifier should be changed to STATIC in add_library
# and PUBLIC elsewhere when the source files will be added
add_library(cyanide INTERFACE)
add_library(cyanide::cyanide ALIAS cyanide)

target_compile_features(cyanide INTERFACE cxx_std_20)
target_include_directories(cyanide INTERFACE "../include")
target_link_libraries(cyanide INTERFACE subhook xbyak::xbyak)

target_sources(cyanide INTERFACE
    "../include/cyanide/func_traits.hpp"
    "../include/cyanide/detour.hpp")
