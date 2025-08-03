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
    about_dialog.cpp
    about_dialog.h
    fn_site_widget.cpp
    fn_site_widget.h
    fnsite_ui.cpp
    fnsite_ui.h
    inventory_model.cpp
    inventory_model.h
    main.cpp
    main_window.cpp
    main_window.h
    mira_map.cpp
    mira_map.h
    precious_resource_ui.cpp
    precious_resource_ui.h
    probe_ui.cpp
    probe_ui.h
    settings.h
    resources/resources.qrc
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
