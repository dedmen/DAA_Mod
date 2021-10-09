cmake_minimum_required (VERSION 3.13)


set(PLUGIN_NAME "daa_x64")

add_definitions(/DNOMINMAX)
add_definitions(/DINTERCEPT_SQF_STRTYPE_RSTRING)
add_definitions(/D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS) #No I don't cheat!

file(GLOB_RECURSE DAASources *.hpp *.cpp *.asm *.s)
SOURCE_GROUP("src" FILES ${DAASources})

set(library_sources ${DAASources})

add_library(${PLUGIN_NAME} SHARED
    ${library_sources}
    #"${PROJECT_SOURCE_DIR}/lib/cpp-base64/base64.cpp"
    #"${PROJECT_SOURCE_DIR}/lib/minilzo/minilzo.c"
)

target_include_directories(${PLUGIN_NAME} PRIVATE
    #"${PROJECT_SOURCE_DIR}/lib/json/single_include/"
    #"${PROJECT_SOURCE_DIR}/lib/cpp-base64/"
    #"${PROJECT_SOURCE_DIR}/lib/minilzo/"
)

target_link_libraries(${PLUGIN_NAME} ${CMAKE_THREAD_LIBS_INIT})

if(CMAKE_COMPILER_IS_GNUCXX)
	add_compile_definitions(__linux__)

	target_compile_options(
		${PLUGIN_NAME} PRIVATE 
		"-std=c++1z"
		"$<$<CONFIG:Release>:-O3>"
		"-s"
		"-fPIC"
		"-fpermissive"
		"-static-libgcc"
		"-static-libstdc++"
		"-march=i686"
		"-m32"
		"-Wno-ignored-attributes"
		"-static"
		"$<$<CONFIG:Release>:-ffunction-sections>"
		"$<$<CONFIG:Release>:-fdata-sections>"

	)
	target_link_options(${PLUGIN_NAME} PRIVATE "-m32" "-fPIC" "-static" "-static-libgcc" "-static-libstdc++"
	"$<$<CONFIG:Release>:-Wl,--gc-sections>"
	"$<$<CONFIG:Release>:-Wl,--strip-all>"
	) 
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

	target_link_libraries(${PLUGIN_NAME} "libstdc++.a")

else()
	target_compile_options(
		${PLUGIN_NAME} PRIVATE 
		"/MP" "/bigobj"
        "$<$<CONFIG:Release>:/Zi>"
        "$<$<CONFIG:Debug>:/ZI>" #Edit and Continue
        "$<$<CONFIG:Debug>:/EDITANDCONTINUE>" #Edit and Continue
		"$<$<CONFIG:Release>:/MT>"
		"$<$<CONFIG:Release>:/Ox>"
		"$<$<CONFIG:Release>:/Ob2>"
		"$<$<CONFIG:Release>:/Oi>"
		"$<$<CONFIG:Release>:/Ot>"
        "/Zc:__cplusplus"
	)
	target_link_options(${PLUGIN_NAME} PRIVATE "/OPT:REF" "/OPT:ICF" "/DEBUG:FULL" "$<$<CONFIG:Debug>:/INCREMENTAL>" "$<$<CONFIG:Debug>:/EDITANDCONTINUE>") 
endif()

#Binary signing
if(EXISTS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sign.bat" AND MSVC)
	add_custom_command(TARGET ${PLUGIN_NAME}
		POST_BUILD
		COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sign.bat
		WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	)
endif()