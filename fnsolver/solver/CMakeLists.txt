set(TARGET solver)

add_library(${TARGET}
    options.cpp
    score_function.cpp
    solution.cpp
    solver.cpp
)

target_link_libraries(${TARGET}
    data
    layout
)

target_compile_features(${TARGET} PUBLIC cxx_std_20)

