cmake_minimum_required(VERSION 3.20)

project(clang-architecture
	LANGUAGES CXX
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
target_compile_definitions(${PROJECT_NAME} PRIVATE
	${LLVM_DEFINITIONS}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
        nlohmann_json::nlohmann_json
        clangAST
        clangIndex
        clangTooling
        LLVM
)
