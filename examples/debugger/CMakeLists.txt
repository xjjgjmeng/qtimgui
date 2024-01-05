set(CMAKE_CXX_STANDARD 17)

set(Bin_Root "${CMAKE_SOURCE_DIR}/Bin")
set(Lib_Root "${CMAKE_SOURCE_DIR}/Lib")

file(GLOB modules "*")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
foreach(m ${modules})
	if(IS_DIRECTORY ${m})
		add_subdirectory(${m})
	endif()
endforeach()