include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG cde26de803964197e7d5e988a17a8dad0cd3721e
)
FetchContent_MakeAvailable(Catch2)

add_executable(cyanide_tests
    "hooks_tests.cpp"
)

target_compile_features(cyanide_tests PRIVATE cxx_std_20)
target_link_libraries(cyanide_tests PRIVATE
    cyanide::cyanide
    Catch2::Catch2WithMain
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(cyanide_tests)
