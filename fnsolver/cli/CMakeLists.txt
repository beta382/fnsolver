set(TARGET ${PROJECT_NAME}-cli)

add_executable(${TARGET}
    cli_options.cpp
    main.cpp
)

target_link_libraries(${TARGET} PRIVATE
    data
    layout
    solver
    util
)

target_compile_features(${TARGET} PRIVATE cxx_std_20)

