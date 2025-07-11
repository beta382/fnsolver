set(TARGET layout)

add_library(${TARGET}
    layout.cpp
    placement.cpp
    resolved_placement.cpp
)

target_link_libraries(${TARGET} PUBLIC
    data
    util
)

target_compile_features(${TARGET} PUBLIC cxx_std_20)

