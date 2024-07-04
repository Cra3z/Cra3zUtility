cmake_minimum_required(VERSION 3.28)

project(use_cpp20_module)

set(CMAKE_CXX_STANDARD 20)

function(enable_import_cxx_std_modules)
    if (MSVC)
        add_library(std_module STATIC)

        set(STD_MODULE_FILES "")

        foreach(MODULE_FILE IN ITEMS "std.ixx" "std.compat.ixx")
            if(NOT EXISTS "${PROJECT_SOURCE_DIR}/cxx-std-modules")
                execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/cxx-std-modules")
            endif()

            set(STD_MODULE_FILE_PATH "$ENV{VCToolsInstallDir}modules/${MODULE_FILE}")

            get_filename_component(MODULE_FILE_NAME "${STD_MODULE_FILE_PATH}" NAME)

            set(STD_MODULE_FILE_IN_PROJECT "${PROJECT_SOURCE_DIR}/cxx-std-modules/${MODULE_FILE_NAME}")

            list(APPEND STD_MODULE_FILES "${STD_MODULE_FILE_IN_PROJECT}")

            if (NOT EXISTS "${STD_MODULE_FILE_IN_PROJECT}")

                get_filename_component(MODULE_NAME "${STD_MODULE_FILE_PATH}" NAME_WLE)

                message(STATUS "Detecting CXX standard module `${MODULE_NAME}` from: ${STD_MODULE_FILE_PATH}")

                file(COPY_FILE "${STD_MODULE_FILE_PATH}" "${STD_MODULE_FILE_IN_PROJECT}")

                message(STATUS "Copy std module file `${MODULE_FILE_NAME}` from: ${STD_MODULE_FILE_PATH}")
            endif()

        endforeach()

        target_sources(
            std_module
            PUBLIC
            FILE_SET CXX_MODULES FILES
            ${STD_MODULE_FILES}
        )
    else()
        message(FATAL_ERROR "Sorry, Cra3zUtility module currently only supports MSVC")
    endif()
endfunction()

enable_import_cxx_std_modules()

list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/../preinstalled/")

find_package(cra3zutil REQUIRED)

add_library(local_cra3zutil STATIC)

target_sources(
    local_cra3zutil
    PUBLIC
    FILE_SET CXX_MODULES
    FILES
    local_cra3zutil.cppm
)

add_executable(
    ${PROJECT_NAME}
    main.cppm
)

target_link_libraries(
    local_cra3zutil
    PRIVATE
    std_module
    cra3zutil::module
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    std_module
    local_cra3zutil
)