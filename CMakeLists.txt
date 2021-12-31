cmake_minimum_required(VERSION 3.19)

project(cyanide VERSION 0.1.0
                DESCRIPTION "Useful utilities and wrappers"
                LANGUAGES CXX)

include(CMakeDependentOption)

option(CYANIDE_FEATURE_ALL "Enable all the library features" ON)

cmake_dependent_option(CYANIDE_FEATURE_MEMORY "Enable memory-related features" ON "CYANIDE_FEATURE_ALL" OFF)
cmake_dependent_option(CYANIDE_FEATURE_HOOK_FRONTEND "Enable hook frontend support" ON "CYANIDE_FEATURE_MEMORY" OFF)
cmake_dependent_option(CYANIDE_FEATURE_HOOK_BACKEND_SUBHOOK "Enable subhook hook backend support" ON "CYANIDE_FEATURE_MEMORY" OFF)
cmake_dependent_option(CYANIDE_FEATURE_SIGNATURE_SCANNER "Enable signature scanner support" ON "CYANIDE_FEATURE_MEMORY" OFF)

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
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(DIRECTORY include/cyanide DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    set(configFile ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake)
    set(versionFile ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake)
    set(configInstallDestination lib/cmake/${PROJECT_NAME})

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
        ${configFile}
        INSTALL_DESTINATION ${configInstallDestination}
    )

    write_basic_package_version_file(
        ${versionFile}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES ${configFile} ${versionFile} DESTINATION ${configInstallDestination})
    install(
        EXPORT ${PROJECT_NAME}Targets
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${configInstallDestination}
    )
endif()

if(CYANIDE_MASTER_PROJECT)
    include(CTest)

    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()
