cmake_minimum_required(VERSION 3.16)

project(RecluseFrameworkTest)

set(RECLUSE_FRAMEWORK_TEST_NAME "RecluseFrameworkTest")

set(TEST_FILES )

set(TEST_FILES ${TEST_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/Memory/MemoryTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MessageBusTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/System/Task.cpp)
set(TEST_INCLUDES ${CMAKE_SOURCE_DIR}/Include)

include(${CMAKE_CURRENT_SOURCE_DIR}/../Build/CMake/GenerateExports.cmake)

# Grab the gtest system.
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.0.zip
)
# For Windows MSVC runtime compatibility
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(${RECLUSE_FRAMEWORK_TEST_NAME} ${TEST_FILES})

ImportThisLibrary(${RECLUSE_FRAMEWORK_TEST_NAME} RecluseFramework ${CMAKE_SOURCE_DIR}/Recluse/Build/Libraries)

target_link_libraries(${RECLUSE_FRAMEWORK_TEST_NAME} PRIVATE gmock_main RecluseFramework)
target_include_directories(${RECLUSE_FRAMEWORK_TEST_NAME} PRIVATE ${TEST_INCLUDES})

# Copy all runtime DLLs required by MyApp to its output directory after building
add_custom_command(TARGET ${RECLUSE_FRAMEWORK_TEST_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${RECLUSE_FRAMEWORK_TEST_NAME}>
        $<TARGET_FILE_DIR:${RECLUSE_FRAMEWORK_TEST_NAME}>
    COMMAND_EXPAND_LISTS
)