set(TARGET ${PROJECT_NAME}-gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 ${Qt6Core_VERSION} REQUIRED COMPONENTS
    Core
    LinguistTools
    Svg
    Widgets
)
qt_standard_project_setup()
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
    abstract_solver_options_widget.h
    constraints_widget.cpp
    constraints_widget.h
    description_widget.cpp
    description_widget.h
    fn_site_widget.cpp
    fn_site_widget.h
    fnsite_ui.cpp
    fnsite_ui.h
    game.h
    game_selector.cpp
    game_selector.h
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
    qicon_from_theme.cpp
    qicon_from_theme.h
    run_dialog.cpp
    run_dialog.h
    run_progress_dialog.cpp
    run_progress_dialog.h
    score_function_widget.cpp
    score_function_widget.h
    settings.h
    solution_widget.cpp
    solution_widget.h
    solver_params_widget.cpp
    solver_params_widget.h
    solver_runner.cpp
    solver_runner.h
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
target_compile_features(${TARGET} PUBLIC cxx_std_20)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources(${TARGET} PRIVATE fnsolver-gui.rc)
endif ()

# When Qt min is 6.7 or higher, this can be replaced with qt_add_translations.
qt_add_lupdate(${TARGET}
    TS_FILES translations/${PROJECT_NAME}_en_US.ts
    OPTIONS -pluralonly -source-language en_US
)
qt_add_lrelease(${TARGET}
    TS_FILES translations/${PROJECT_NAME}_en_US.ts
    QM_FILES_OUTPUT_VARIABLE QM_FILES
)
qt_add_resources(${TARGET} "translations"
    PREFIX "/i18n"
    BASE "${CMAKE_CURRENT_BINARY_DIR}"
    FILES "${QM_FILES}"
)

install(
    TARGETS ${TARGET}
    COMPONENT gui
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(
    FILES "$<TARGET_RUNTIME_DLLS:${TARGET}>"
    COMPONENT gui
    TYPE BIN
)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    qt_generate_deploy_app_script(
        TARGET ${TARGET}
        OUTPUT_SCRIPT DEPLOY_SCRIPT
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(
        SCRIPT ${DEPLOY_SCRIPT}
        COMPONENT gui
    )
endif ()
