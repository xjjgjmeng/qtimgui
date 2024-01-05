set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(ProId FourPaneViewer)
set(CMAKE_CXX_STANDARD 17)

file(GLOB   header_file	    "*.h" "*.hpp")
file(GLOB   source_file     "*.cpp" "*.cxx")
file(GLOB   ui_file         "*.ui")
file(GLOB   resource_file   "*.qrc")
add_executable(${ProId} WIN32 ${header_file} ${source_file} ${ui_file} ${resource_file})

# vtk9.0.3
find_package(VTK REQUIRED)
target_link_libraries(${ProId} PRIVATE qt_imgui_widgets ${VTK_LIBRARIES})
target_link_libraries(${ProId} PRIVATE implot)

vtk_module_autoinit(
TARGETS ${ProId}
MODULES ${VTK_LIBRARIES}
)

set(Bin_Root "${CMAKE_SOURCE_DIR}/Bin")
set(Lib_Root "${CMAKE_SOURCE_DIR}/Lib")
set_target_properties(${ProId}
PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${Bin_Root}
ARCHIVE_OUTPUT_DIRECTORY ${Lib_Root}
VS_DEBUGGER_WORKING_DIRECTORY ${Bin_Root}/$<CONFIG>
)