cmake_minimum_required(VERSION 3.28)

project(cra3zutil)

set(CMAKE_CXX_STANDARD 20)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/permissive->")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>")

add_library(
    header
    INTERFACE
)

add_library(
    module
    INTERFACE
)

target_include_directories(
    header
    INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${PROJECT_NAME}/include>
)

target_include_directories(
    module
    INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include_module>
    $<INSTALL_INTERFACE:${PROJECT_NAME}/include_module>
)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include
    DESTINATION ${PROJECT_NAME}
)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include_module
    DESTINATION ${PROJECT_NAME}
)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/modules
    DESTINATION ${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)

foreach(FEATURE IN ITEMS header module)

    install(
        TARGETS ${FEATURE}
        EXPORT ${FEATURE}Targets
    )

    install(
        EXPORT ${FEATURE}Targets
        FILE ${FEATURE}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${PROJECT_NAME}/lib/cmake/${PROJECT_NAME}
    )

endforeach()

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${PROJECT_NAME}/lib/cmake/${PROJECT_NAME}
)

install(
    FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    DESTINATION ${PROJECT_NAME}/lib/cmake/${PROJECT_NAME}
)