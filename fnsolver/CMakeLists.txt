configure_file(fnsolver_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/fnsolver_config.h ESCAPE_QUOTES)

add_subdirectory(cli)
add_subdirectory(data)
add_subdirectory(layout)
add_subdirectory(solver)
add_subdirectory(util)
add_subdirectory(gui)
