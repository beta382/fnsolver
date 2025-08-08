set(TARGET ${PROJECT_NAME}-gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core
    Svg
    Widgets
)
include(FetchContent)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

qt_add_executable(${TARGET} WIN32
    about_dialog.cpp
    about_dialog.h
    constraints_widget.cpp
    constraints_widget.h
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
    options_loader.cpp
    options_loader.h
    precious_resource_ui.cpp
    precious_resource_ui.h
    probe_ui.cpp
    probe_ui.h
    run_dialog.cpp
    run_dialog.h
    score_function_widget.cpp
    score_function_widget.h
    settings.h
    solution_widget.cpp
    solution_widget.h
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
    tomlplusplus::tomlplusplus
)
target_compile_definitions(${TARGET} PRIVATE QT_NO_KEYWORDS)

install(
    TARGETS ${TARGET}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(
    FILES "$<TARGET_RUNTIME_DLLS:${TARGET}>"
    TYPE BIN
)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    qt_generate_deploy_app_script(
        TARGET ${TARGET}
        OUTPUT_SCRIPT DEPLOY_SCRIPT
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${DEPLOY_SCRIPT})
endif ()
