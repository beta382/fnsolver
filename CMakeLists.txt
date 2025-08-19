cmake_minimum_required(VERSION 3.28)
project(fnsolver
    VERSION 1.1.0
    DESCRIPTION "A tool for \"Xenoblade Chronicles X\" and \"Xenoblade Chronicles X: Definitive Edition\", which generates tailored solutions to FrontierNav layouts."
    LANGUAGES CXX
    HOMEPAGE_URL "https://github.com/beta382/fnsolver"
)
set(PROJECT_DISPLAY_NAME "FnSolver")
set(PROJECT_AUTHOR "beta382")
set(PROJECT_ORGANIZATION_NAME "${PROJECT_AUTHOR}")
set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(fnsolver)

option(BUILD_PACKAGE Off)
if (BUILD_PACKAGE)
  add_subdirectory(install)
endif ()
