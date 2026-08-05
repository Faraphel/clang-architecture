cmake_minimum_required(VERSION 3.25)

project(clang-architecture
    VERSION 1.0.0
    DESCRIPTION "A tool based on clang capable of parsing the symbolics relations in a C/C++ project into a JSON file"
    HOMEPAGE_URL "https://git.faraphel.fr/faraphel/clang-architecture"
    LANGUAGES C CXX
)

# Options

option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_PROFILING "Enable profiling" OFF)
option(ENABLE_TESTS "Enable tests" OFF)


add_library(${PROJECT_NAME}-instrumentation INTERFACE)

if(ENABLE_COVERAGE)
        target_compile_options(${PROJECT_NAME}-instrumentation INTERFACE
                --coverage
        )
        target_link_options(${PROJECT_NAME}-instrumentation INTERFACE
                --coverage
        )
endif()

if(ENABLE_PROFILING)
        target_compile_options(${PROJECT_NAME}-instrumentation INTERFACE
                -pg
        )
        target_link_options(${PROJECT_NAME}-instrumentation INTERFACE
                -pg
        )
endif()

# Dependencies

include(GNUInstallDirs)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
add_subdirectory(./external/json SYSTEM)

message(STATUS "LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Build

add_library(${PROJECT_NAME}-library STATIC
        ./source/SymbolActionFactory.cpp
        ./source/SymbolConsumer.cpp
)
target_include_directories(${PROJECT_NAME}-library PUBLIC
        ./source
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME}-library PUBLIC
        ${PROJECT_NAME}-instrumentation
        nlohmann_json::nlohmann_json
        clang-cpp
        LLVM
)
set_target_properties(${PROJECT_NAME}-library PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

add_executable(${PROJECT_NAME}
	    ./source/main.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE
        ${PROJECT_NAME}-instrumentation
        ${PROJECT_NAME}-library
)
set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

# Install

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Tests

if(ENABLE_TESTS)
    find_package(Catch2 3 REQUIRED)

    include(CTest)
    include(Catch)

    add_executable(${PROJECT_NAME}-tests
            tests/01_simple_function.cpp
    )
    target_link_libraries(${PROJECT_NAME}-tests PRIVATE
            ${PROJECT_NAME}-instrumentation
            ${PROJECT_NAME}-library
            Catch2::Catch2WithMain
    )
    catch_discover_tests(${PROJECT_NAME}-tests)
endif()

# Package

set(CPACK_GENERATOR "DEB;RPM;TGZ;ZIP")

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "faraphel@faraphel.fr")

# apt specific package configuration
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "clang, libclang-dev, llvm, llvm-dev")
set(CPACK_DEBIAN_PACKAGE_RELEASE 1)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

# rpm specific package configuration
set(CPACK_RPM_PACKAGE_LICENSE "CC ANSA 4.0")
set(CPACK_RPM_PACKAGE_GROUP "Development/Tools")
set(CPACK_RPM_PACKAGE_REQUIRES "clang, clang-devel, llvm, llvm-devel")
set(CPACK_RPM_PACKAGE_RELEASE 1)
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)

# run CPack
include(CPack)
