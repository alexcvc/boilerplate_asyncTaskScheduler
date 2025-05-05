find_package(Catch2 REQUIRED)

set(TargetName test_scheduler)

# enable testing
enable_testing()

# Add test target
add_executable(${TargetName} testCases.cpp
   ${CMAKE_SOURCE_DIR}/include/iController.hpp
   ${CMAKE_SOURCE_DIR}/include/iUserData.hpp
   ${CMAKE_SOURCE_DIR}/include/event.hpp
   ${CMAKE_SOURCE_DIR}/include/scheduler.hpp
   catch_main.cpp
)

# add compiler definition
target_include_directories(${TargetName} PUBLIC ${CMAKE_SOURCE_DIR}/include)

## link with library
#  target_link_libraries(${TargetName} gtest gtest_main)
target_link_libraries(${TargetName} PRIVATE Catch2::Catch2)

# Add the test to CTest
add_test(NAME ${TargetName} COMMAND ${TargetName})
