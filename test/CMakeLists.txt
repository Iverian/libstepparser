enable_testing()


file(GLOB TEST_SOURCES "src/test_*.cpp")

###################################################################################################
# Make all tests in sigle executable
add_executable(all_tests
    "src/all_tests.cpp"
    ${TEST_SOURCES}
)

target_include_directories(all_tests
    PRIVATE
        "../src"
        ${CONAN_INCLUDE_DIRS_GTEST}
)

target_link_libraries(all_tests
    PRIVATE
        GeomModel::GeomModel
        CONAN_PKG::gtest
)

add_test(all_tests all_tests)
###################################################################################################

###################################################################################################
# Make separate tests
foreach(_TEST_FILE ${TESTS_SOURCES})
    get_filename_component(_TEST_NAME ${_TEST_FILE} NAME_WE)
    add_executable(${_TEST_NAME} ${_TEST_FILE})

    target_include_directories(${TESTS}
        PRIVATE
            "../src"
            ${CONAN_INCLUDE_DIRS_GTEST}
    )

    target_link_libraries(${_TEST_NAME}
        PRIVATE
            GeomModel::GeomModel
            CONAN_PKG::gtest
    )

    add_test(${_TEST_NAME} ${_TEST_NAME})
endforeach()
###################################################################################################
