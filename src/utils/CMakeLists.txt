# utils/CMakeLists.txt
# 工具类
qt_add_library(WebNav_utils STATIC
    PlatformUtils.h    PlatformUtils.cpp
    ColorUtils.h       ColorUtils.cpp
    ImageUtils.h       ImageUtils.cpp
)
target_link_libraries(WebNav_utils PUBLIC
    Qt6::Core
    Qt6::Widgets
)
target_include_directories(WebNav_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
