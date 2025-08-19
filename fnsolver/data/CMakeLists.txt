set(TARGET data)

add_library(${TARGET} STATIC
    fnsite.cpp
    precious_resource.cpp
    probe.cpp
    resource_yield.cpp
)

target_compile_features(${TARGET} PUBLIC cxx_std_20)

