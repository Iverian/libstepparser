cmake_minimum_required(VERSION 3.10)

set(PROJECT_TARGET stepparse)
set(PROJECT_SHORT_NAME stp)
string(TOUPPER ${PROJECT_TARGET} PROJECT_TARGET_UPPER)
string(TOUPPER ${PROJECT_SHORT_NAME} PROJECT_SHORT_NAME_UPPER)

project(libstepparser VERSION 2.0.0 LANGUAGES CXX)

option(STP_ENABLE_TESTS "Enable unit tests" YES)

# Include additional CMake packages
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
#

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake")

# Configure dependencies
find_package(fmt CONFIG REQUIRED)
find_package(commons CONFIG REQUIRED)
find_package(geommodel CONFIG REQUIRED)
#

# Find source files
file(GLOB_RECURSE
  PROJECT_SOURCES
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)
file(GLOB_RECURSE
  PROJECT_PUBLIC_HEADERS
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
  "${CMAKE_CURRENT_BINARY_DIR}/exports.hpp"
 )
#

# Create target and set properties
add_library(${PROJECT_TARGET})

target_sources(${PROJECT_TARGET}
  PRIVATE
    ${PROJECT_SOURCES}
    ${PROJECT_PUBLIC_HEADERS}
)
target_include_directories(${PROJECT_TARGET}
  PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  PRIVATE
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR};${PROJECT_SOURCE_DIR}/src>"
)
target_compile_features(${PROJECT_TARGET}
  PUBLIC
    cxx_std_17
)
target_compile_definitions(${PROJECT_TARGET}
  PUBLIC
    $<BUILD_INTERFACE:_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS>
)
target_link_libraries(${PROJECT_TARGET}
  PRIVATE
    commons::commons
  PUBLIC
    geommodel::geommodel
)
set_target_properties(${PROJECT_TARGET}
  PROPERTIES
    DEBUG_POSTFIX d
    CXX_EXTENSIONS NO
    WINDOWS_EXPORT_ALL_SYMBOLS ${STP_ENABLE_TESTS}
)

include(GenerateExportHeader)
generate_export_header(${PROJECT_TARGET}
  BASE_NAME "${PROJECT_SHORT_NAME_UPPER}"
  EXPORT_FILE_NAME "exports.hpp"
  NO_DEPRECATED_MACRO_NAME "${PROJECT_SHORT_NAME_UPPER}_OMIT_DEPRECATED"
  DEFINE_NO_DEPRECATED
)
if (STP_ENABLE_TESTS)
  target_compile_definitions(${PROJECT_TARGET}
    PUBLIC
      "${PROJECT_SHORT_NAME_UPPER}_STATIC_DEFINE"
  )
endif()

add_library(${PROJECT_TARGET}::${PROJECT_TARGET} ALIAS ${PROJECT_TARGET})
#

install(TARGETS ${PROJECT_TARGET}
  EXPORT ${PROJECT_TARGET}_targets
  RUNTIME
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  INCLUDES
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
install(
  DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
install(
  FILES "${PROJECT_BINARY_DIR}/exports.hpp"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_SHORT_NAME}"
)

set(INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_TARGET}")
install(EXPORT ${PROJECT_TARGET}_targets
  DESTINATION "${INSTALL_CONFIGDIR}"
  NAMESPACE "${PROJECT_TARGET}::"
  FILE "${PROJECT_TARGET}Targets.cmake"
)

get_target_property(PROJECT_TARGET_TYPE ${PROJECT_TARGET} TYPE)
if (WIN32 AND
    PROJECT_TARGET_TYPE STREQUAL "SHARED_LIBRARY" AND
    NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
  install(
    FILES "$<TARGET_PDB_FILE:${PROJECT_TARGET}>"
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
    OPTIONAL
  )
endif()

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_TARGET}ConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_TARGET}Config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_TARGET}Config.cmake"
  INSTALL_DESTINATION "${INSTALL_CONFIGDIR}"
)

# Install the config, configversion and custom find modules
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_TARGET}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_TARGET}ConfigVersion.cmake"
  DESTINATION "${INSTALL_CONFIGDIR}"
)
#

# Add tests
if (STP_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
#
