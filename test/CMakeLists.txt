find_package(Catch2 REQUIRED)

# enable testing
enable_testing()

# Add the test sources
# Add the test sources explicitly
set(SOURCE_FILES
   test_1.cpp
)


# For each source file a test target is created
foreach (SOURCE_PATH ${SOURCE_FILES})

  get_filename_component(TargetName ${SOURCE_PATH} NAME_WE)

  # Add test target
  add_executable(${TargetName} ${SOURCE_PATH}
     ${CMAKE_SOURCE_DIR}/include/iController.hpp
     ${CMAKE_SOURCE_DIR}/include/iUserData.hpp
     ${CMAKE_SOURCE_DIR}/include/event.hpp
     ${CMAKE_SOURCE_DIR}/include/scheduler.hpp
  )

  # add compiler definition
  target_include_directories(${TargetName} PUBLIC ${CMAKE_SOURCE_DIR}/include)

  ## link with library
  #  target_link_libraries(${TargetName} gtest gtest_main)
  target_link_libraries(${TargetName} PRIVATE Catch2::Catch2)

  # Add the test to CTest
  add_test(NAME ${TargetName} COMMAND ${TargetName})

endforeach ()