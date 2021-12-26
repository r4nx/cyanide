cmake_minimum_required(VERSION 3.17.1)

project(cyanide VERSION 0.1.0
                DESCRIPTION "Useful utilities and wrappers"
                LANGUAGES CXX)

set(SUBHOOK_STATIC ON CACHE BOOL "" FORCE)

include(FetchContent)

FetchContent_Declare(
    subhook
    GIT_REPOSITORY https://github.com/Zeex/subhook.git
    GIT_TAG 284047b761bd61e3c0518b0be235aa77172b1010
)

FetchContent_Declare(
    xbyak
    GIT_REPOSITORY https://github.com/herumi/xbyak
    GIT_TAG a34850b2df66a186c8d947b4d72acc839926321f
)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG 023b5306b444ebe3c20a008cdd150cf5510ff36d)

FetchContent_MakeAvailable(subhook)
FetchContent_MakeAvailable(xbyak)
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")

add_subdirectory(src)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    include(CTest)
    add_subdirectory(tests)
endif()
