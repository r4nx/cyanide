include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG 65cc7fd2ae39a7a543767f661b096d7d521ee4f0
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
