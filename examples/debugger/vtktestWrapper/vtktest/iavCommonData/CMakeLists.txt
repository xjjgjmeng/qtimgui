set(CMAKE_CXX_STANDARD 17)

set(ProId iavCommonData)

file(GLOB_RECURSE   header_file	    "*.h")
file(GLOB           source_file     "*.cpp")

add_library(${ProId} SHARED ${header_file} ${source_file})

set_target_properties(${ProId}
PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${Bin_Root}
ARCHIVE_OUTPUT_DIRECTORY ${Lib_Root}
COMPILE_DEFINITIONS "IAVCOMMONDATA_DLL_EXPORTS"
)
target_include_directories(${ProId} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})