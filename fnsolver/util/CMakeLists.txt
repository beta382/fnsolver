set(TARGET util)

add_library(${TARGET} INTERFACE)

target_compile_features(${TARGET} INTERFACE cxx_std_20)

