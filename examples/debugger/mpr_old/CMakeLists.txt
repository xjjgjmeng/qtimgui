set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(ProId debugger)
set(CMAKE_CXX_STANDARD 17)

if (ANDROID)
    add_library(qt_imgui_demo_widget SHARED demo-widget.cpp)
else()
    file(GLOB_RECURSE header_file "*.h")
    file(GLOB source_file "main.cpp" "*.cpp")
    add_executable(debugger ${header_file} ${source_file})
endif()
# vtk9.0.3
find_package(VTK REQUIRED)
target_link_libraries(debugger PRIVATE qt_imgui_widgets ${VTK_LIBRARIES})
target_link_libraries(debugger PRIVATE implot)

vtk_module_autoinit(
TARGETS ${ProId}
MODULES ${VTK_LIBRARIES}
)

if(ANDROID)
    include(${CMAKE_CURRENT_LIST_DIR}/../../tools/qt-android-cmake/AddQtAndroidApk.cmake)
    add_qt_android_apk(qt_imgui_demo_widget_apk qt_imgui_demo_widget)
endif()

set(Bin_Root "${CMAKE_SOURCE_DIR}/Bin")
set(Lib_Root "${CMAKE_SOURCE_DIR}/Lib")
set_target_properties(${ProId}
PROPERTIES
RUNTIME_OUTPUT_DIRECTORY ${Bin_Root}
ARCHIVE_OUTPUT_DIRECTORY ${Lib_Root}
VS_DEBUGGER_WORKING_DIRECTORY ${Bin_Root}/$<CONFIG>
)