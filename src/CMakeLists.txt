include(FetchContent)
include(GNUInstallDirs)

add_library(cyanide STATIC)
add_library(cyanide::cyanide ALIAS cyanide)

target_compile_features(cyanide PUBLIC cxx_std_20)
target_include_directories(cyanide PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(CYANIDE_FEATURE_HOOK)
	FetchContent_Declare(
		xbyak
		GIT_REPOSITORY https://github.com/herumi/xbyak.git
		GIT_TAG 2a85bba3fe304c387d652537b715b09005e747f9
	)

	FetchContent_Declare(
		polyhook
		GIT_REPOSITORY https://github.com/stevemk14ebr/PolyHook_2_0.git
		GIT_TAG ffb7685f0acce29de5944d223bf7bc7b2ddb647c
	)

	FetchContent_MakeAvailable(xbyak)
	FetchContent_MakeAvailable(polyhook)

	target_link_libraries(cyanide PUBLIC xbyak::xbyak PolyHook_2)
endif()

target_sources(cyanide PRIVATE
	"main.cpp"
)
