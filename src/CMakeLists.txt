include(GNUInstallDirs)

add_library(cyanide STATIC)
add_library(cyanide::cyanide ALIAS cyanide)

target_compile_features(cyanide PUBLIC cxx_std_20)
target_include_directories(cyanide PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Resolve dependencies according to enabled features
include(FetchContent)

if(CYANIDE_HOOKS_SUPPORT)
	FetchContent_Declare(
		xbyak
		GIT_REPOSITORY https://github.com/herumi/xbyak.git
		GIT_TAG 2a85bba3fe304c387d652537b715b09005e747f9
	)

	FetchContent_MakeAvailable(xbyak)

	target_link_libraries(cyanide PUBLIC xbyak::xbyak)
endif()

target_sources(cyanide PRIVATE
	"main.cpp"
)
