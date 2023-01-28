cmake_minimum_required(VERSION 3.19)

project(cyanide VERSION 0.1.0
                DESCRIPTION "Useful utilities and wrappers"
                LANGUAGES CXX)

include(CMakeDependentOption)

option(CYANIDE_FEATURE_ALL "Enable all the library features" ON)

cmake_dependent_option(
    CYANIDE_FEATURE_HOOK
    "Enable hooks support" ON
    "CYANIDE_FEATURE_ALL" OFF
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CYANIDE_MASTER_PROJECT ON)
endif()

add_subdirectory(src)

option(CYANIDE_INSTALL "Generate the install targets" ${CYANIDE_MASTER_PROJECT})
if(CYANIDE_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(
        TARGETS cyanide
        EXPORT ${PROJECT_NAME}-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(DIRECTORY include/cyanide DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    set(config_file ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake)
    set(version_file ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake)
    set(config_install_destination lib/cmake/${PROJECT_NAME})

    set(XBYAK_CONFIG_DIR "lib/cmake/xbyak")
    set(POLYHOOK_CONFIG_DIR "lib/PolyHook_2")

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in
        ${config_file}
        INSTALL_DESTINATION ${config_install_destination}
        PATH_VARS
            XBYAK_CONFIG_DIR
            POLYHOOK_CONFIG_DIR
    )

    write_basic_package_version_file(
        ${version_file}
        COMPATIBILITY SameMajorVersion
    )

    install(
        FILES ${config_file} ${version_file}
        DESTINATION ${config_install_destination}
    )

    install(
        EXPORT ${PROJECT_NAME}-targets
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${config_install_destination}
    )
endif()

if(CYANIDE_MASTER_PROJECT)
    include(CTest)

    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()
