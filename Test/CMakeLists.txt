cmake_minimum_required(VERSION 3.16)

project(RecluseFrameworkTest)

set(RECLUSE_FRAMEWORK_TEST_NAME "RecluseFrameworkTest")

set(TEST_FILES )

set(TEST_FILES ${TEST_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/Memory/MemoryTest.cpp)
set(TEST_INCLUDES ${CMAKE_SOURCE_DIR}/Include)

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

target_link_libraries(${RECLUSE_FRAMEWORK_TEST_NAME} PRIVATE gtest_main RecluseFramework)
target_include_directories(${RECLUSE_FRAMEWORK_TEST_NAME} PRIVATE ${TEST_INCLUDES})