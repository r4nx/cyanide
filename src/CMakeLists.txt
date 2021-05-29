# add_library(cyanide SHARED)
# add_library(cyanide::cyanide ALIAS cyanide)
add_executable(cyanide)

target_compile_features(cyanide PUBLIC cxx_std_20)
target_include_directories(cyanide PUBLIC "../include")
target_link_libraries(cyanide PUBLIC subhook xbyak::xbyak)

target_sources(cyanide PUBLIC
    "main.cpp")
