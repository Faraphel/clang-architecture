cmake_minimum_required(VERSION 3.20)

project(clang-architecture
    VERSION 1.0.0
    DESCRIPTION "A tool based on clang capable of parsing the symbolics relations in a C/C++ project into a JSON file"
    HOMEPAGE_URL "https://git.faraphel.fr/faraphel/clang-architecture"
    LANGUAGES C CXX
)

# Dependencies

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
add_subdirectory(./external/json)

message(STATUS "LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Build

add_executable(${PROJECT_NAME}
	./source/main.cpp
	./source/SymbolActionFactory.cpp
	./source/SymbolConsumer.cpp
)
set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)
target_include_directories(${PROJECT_NAME} PRIVATE
	${LLVM_INCLUDE_DIRS}
	${CLANG_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
        nlohmann_json::nlohmann_json
        clangAST
        clangIndex
        clangTooling
        LLVM
)

# Install

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Package

set(CPACK_GENERATOR "DEB;RPM")

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