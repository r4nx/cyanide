add_executable(tests
    "hooks.cpp")

target_compile_features(tests PRIVATE cxx_std_20)
# target_link_libraries(tests PRIVATE cyanide Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(tests)
