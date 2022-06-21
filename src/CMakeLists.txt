include(GNUInstallDirs)

add_library(cyanide STATIC)
add_library(cyanide::cyanide ALIAS cyanide)

target_compile_features(cyanide PUBLIC cxx_std_23)
target_include_directories(cyanide PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(CYANIDE_FEATURE_MEMORY)
	add_subdirectory(memory)
endif()

target_sources(cyanide PRIVATE
	"main.cpp"
)
