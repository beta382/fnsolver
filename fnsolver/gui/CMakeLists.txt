set(TARGET ${PROJECT_NAME}-gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core
    Svg
    Widgets
)

qt_add_executable(${TARGET} WIN32
    resources/resources.qrc
    main.cpp
    about_dialog.h
    about_dialog.cpp
    fn_site_widget.h
    fn_site_widget.cpp
    fnsite_ui.h
    fnsite_ui.cpp
    main_window.h
    main_window.cpp
    mira_map.h
    mira_map.cpp
    precious_resource_ui.h
    precious_resource_ui.cpp
    probe_ui.h
    probe_ui.cpp
    settings.h
)
target_link_libraries(${TARGET} PRIVATE
    data
    layout
    solver
    util
    Qt::Core
    Qt::Widgets
    Qt::Svg
)
target_compile_definitions(${TARGET} PRIVATE QT_NO_KEYWORDS)
