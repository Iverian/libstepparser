cmake_minimum_required(VERSION 3.0)
project(libstepparser VERSION 1.0.0 LANGUAGES CXX)

###################################################################################################
# Include additional cmake packages
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
###################################################################################################

set(CMAKE_CXX_STANDARD 17)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

###################################################################################################
# Configure conan
if(NOT EXISTS ${CMAKE_BINARY_DIR}/conan.cmake)
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(
    REQUIRES gtest/1.8.1@bincrafters/stable
             fmt/5.2.1@bincrafters/stable
    BASIC_SETUP CMAKE_TARGETS
    BUILD missing
)
###################################################################################################

###################################################################################################
# Declare dependencies
find_package(GeomModel REQUIRED)
###################################################################################################

###################################################################################################
# Find source files
file(GLOB MAIN_HEADERS "include/step_parser.h")

set(MAIN_SOURCES "")
file(GLOB _SRC_CHILDREN "src/*")

foreach(_CHILD ${_SRC_CHILDREN})
    if(IS_DIRECTORY ${_CHILD})
        file(GLOB _CHILD_HEADERS "${_CHILD}/*.h")
	    file(GLOB _CHILD_SOURCES "${_CHILD}/*.cpp")
        list(APPEND MAIN_SOURCES ${_CHILD_HEADERS})
        list(APPEND MAIN_SOURCES ${_CHILD_SOURCES})
	endif()
endforeach()
###################################################################################################

###################################################################################################
# Create target and set properties
if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

add_library(step_parser SHARED ${MAIN_HEADERS} ${MAIN_SOURCES})

target_include_directories(step_parser
    PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_FULL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CONAN_INCLUDE_DIRS_FMT}
        "src"
)

target_link_libraries(step_parser
    PUBLIC
        GeomModel::GeomModel
    PRIVATE
        CONAN_PKG::fmt
)
###################################################################################################

###################################################################################################
# Install package
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/step_parser)

install(TARGETS step_parser
    EXPORT step_parser-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set_target_properties(step_parser PROPERTIES EXPORT_NAME StepParser)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT step_parser-targets
    FILE StepParserTargets.cmake
    NAMESPACE StepParser::
    DESTINATION ${INSTALL_CONFIGDIR}
)

install(
    FILES $<TARGET_PDB_FILE:step_parser>
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    OPTIONAL
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/StepParserConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/StepParserConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/StepParserConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

#Install the config, configversion and custom find modules
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/StepParserConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/StepParserConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)
###################################################################################################

export(
    EXPORT step_parser-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/StepParserTargets.cmake
    NAMESPACE StepParser::
)

#Register package in user's package registry
export(PACKAGE StepParser)

###################################################################################################
# Add tests
add_subdirectory(test)
###################################################################################################
