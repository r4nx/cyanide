include(FetchContent)

if(CYANIDE_FEATURE_HOOK_FRONTEND)
	FetchContent_Declare(
		xbyak
		GIT_REPOSITORY https://github.com/herumi/xbyak.git
		GIT_TAG 2a85bba3fe304c387d652537b715b09005e747f9
	)

	FetchContent_MakeAvailable(xbyak)

	target_link_libraries(cyanide PUBLIC xbyak::xbyak)
endif()

if(CYANIDE_FEATURE_HOOK_BACKEND_SUBHOOK)
	set(SUBHOOK_STATIC ON)

	FetchContent_Declare(
		subhook
		GIT_REPOSITORY https://github.com/r4nx/subhook.git
		GIT_TAG 00ea69cbd01521f10a6c1488c115644d00e64b79
	)

	FetchContent_MakeAvailable(subhook)

	target_link_libraries(cyanide PUBLIC subhook)

	target_sources(cyanide PRIVATE
		"hook_backend_subhook.cpp"
	)
endif()

if(CYANIDE_FEATURE_SIGNATURE_SCANNER)
	if(WIN32)
		target_sources(cyanide PRIVATE
			"signature_scanner_win.cpp"
		)
	else()
		message(FATAL_ERROR "Signature scanner isn't implemented for any platform other than Windows")
	endif()
endif()
