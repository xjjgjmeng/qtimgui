#cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_CXX_STANDARD 17)

#set(Bin_Root "${CMAKE_CURRENT_SOURCE_DIR}/Bin")
#set(Lib_Root "${CMAKE_CURRENT_SOURCE_DIR}/Lib")

#project(Widget)
set(ProId VtkTest)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
# set(Qt5_DIR "C:/Qt/Qt5.12.4/5.12.4/msvc2017_64/lib/cmake/Qt5")
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(VTK REQUIRED)

add_subdirectory(iavCommonData)
add_subdirectory(iavTMJViewer)
add_subdirectory(iavCPRViewer)

file(GLOB			header_file	    "*.h")
file(GLOB           source_file     "*.cpp")

add_executable(${ProId} ${header_file} ${source_file})
target_link_libraries(${ProId} Qt5::Widgets ${VTK_LIBRARIES} iavTMJViewer iavCPRViewer iavCommonData)

set_target_properties(${ProId}
PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${Bin_Root}
ARCHIVE_OUTPUT_DIRECTORY ${Lib_Root}
)