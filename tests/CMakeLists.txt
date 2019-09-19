enable_testing()
include(GoogleTest)
find_package(GTest MODULE REQUIRED)

file(GLOB_RECURSE
  ALLTESTS_SOURCES
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/test_*.cpp"
)

if (ALLTESTS_SOURCES)
  add_executable(alltests)

  target_sources(alltests
    PRIVATE
      ${ALLTESTS_SOURCES}
  )
  target_include_directories(alltests
    PRIVATE
      "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR};${PROJECT_SOURCE_DIR}/src>"
  )
  target_link_libraries(alltests
    PRIVATE
      stepparse::stepparse
      fmt::fmt
      GTest::GTest
      GTest::Main
  )
  set_target_properties(alltests
    PROPERTIES
      DEBUG_POSTFIX d
      CXX_EXTENSIONS NO
      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
  )
  gtest_discover_tests(alltests
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )
endif()
