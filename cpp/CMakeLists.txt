cmake_minimum_required(VERSION 3.10)

project(memilio VERSION 0.1.0)

option(MEMILIO_BUILD_TESTS "Build memilio unit tests." ON)
option(MEMILIO_BUILD_EXAMPLES "Build memilio examples." ON)
option(MEMILIO_BUILD_MODELS "Build memilio models." ON)
option(MEMILIO_BUILD_SIMULATIONS "Build memilio simulations that were used for scientific articles." ON)
option(MEMILIO_USE_BUNDLED_SPDLOG "Use spdlog bundled with epi" ON)
option(MEMILIO_USE_BUNDLED_EIGEN "Use eigen bundled with epi" ON)
option(MEMILIO_USE_BUNDLED_BOOST "Use boost bundled with epi (only for epi-io)" ON)
option(MEMILIO_USE_BUNDLED_JSONCPP "Use jsoncpp bundled with epi (only for epi-io)" ON)
option(MEMILIO_SANITIZE_ADDRESS "Enable address sanitizer." OFF)
option(MEMILIO_SANITIZE_UNDEFINED "Enable undefined behavior sanitizer." OFF)

mark_as_advanced(MEMILIO_USE_BUNDLED_SPDLOG MEMILIO_SANITIZE_ADDRESS MEMILIO_SANITIZE_UNDEFINED)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# code coverage analysis
# Note: this only works under linux and with make
# Ninja creates different directory names which do not work together with this scrupt
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    option (MEMILIO_TEST_COVERAGE "Enable GCov coverage analysis (adds a 'coverage' target)" OFF)
    mark_as_advanced(MEMILIO_TEST_COVERAGE)
    if (MEMILIO_TEST_COVERAGE)
        message(STATUS "Coverage enabled")
        include(CodeCoverage)
        append_coverage_compiler_flags()
        setup_target_for_coverage_lcov(
            NAME coverage
            EXECUTABLE memilio-test
            EXCLUDE "${CMAKE_SOURCE_DIR}/tests*" "${CMAKE_SOURCE_DIR}/simulations*" "${CMAKE_SOURCE_DIR}/examples*" "${CMAKE_BINARY_DIR}/*" "/usr*"
        )
    endif()
endif()

# set sanitizer compiler flags
if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7))
    if(MEMILIO_SANITIZE_ADDRESS)
        string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=address")
        string(APPEND CMAKE_LINKER_FLAGS_DEBUG  " -fsanitize=address")
    endif(MEMILIO_SANITIZE_ADDRESS)

    if(MEMILIO_SANITIZE_UNDEFINED)
        string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=undefined")
        string(APPEND CMAKE_LINKER_FLAGS_DEBUG  " -fsanitize=undefined")
    endif(MEMILIO_SANITIZE_UNDEFINED)
    
    if(MEMILIO_SANITIZE_ADDRESS OR MEMILIO_SANITIZE_UNDEFINED)
        string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fno-omit-frame-pointer -fno-sanitize-recover=all")
        string(APPEND CMAKE_LINKER_FLAGS_DEBUG  " -fno-omit-frame-pointer -fno-sanitize-recover=all")
    endif(MEMILIO_SANITIZE_ADDRESS OR MEMILIO_SANITIZE_UNDEFINED)
endif((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7))

# define flags to enable most warnings and treat them as errors for different compilers
# add flags to each target separately instead of globally so users have the choice to use their own flags
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")    
    set(MEMILIO_CXX_FLAGS_ENABLE_WARNING_ERRORS
        "-Wno-unknown-warning;-Wno-pragmas;-Wall;-Wextra;-Werror;-Wshadow;--pedantic-errors;-Wno-deprecated-copy")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(MEMILIO_CXX_FLAGS_ENABLE_WARNING_ERRORS
        "-Wno-unknown-warning-option;-Wall;-Wextra;-Werror;-Wshadow;--pedantic-errors;-Wno-deprecated;-Wno-gnu-zero-variadic-macro-arguments")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(MEMILIO_CXX_FLAGS_ENABLE_WARNING_ERRORS
        "/W4;/WX")
endif()

# add parts of the project
include(thirdparty/CMakeLists.txt)
add_subdirectory(memilio)
if (MEMILIO_BUILD_MODELS)
    add_subdirectory(models/abm)
    add_subdirectory(models/secir)
    add_subdirectory(models/seir)
endif()
if (MEMILIO_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
if (MEMILIO_BUILD_TESTS)
    add_subdirectory(tests)
endif()
if (MEMILIO_BUILD_SIMULATIONS)
    add_subdirectory(simulations)
endif()

# install
include(GNUInstallDirs)

install(TARGETS memilio
        EXPORT memilio-targets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY memilio DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN memilio/*/*.h)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/memilio DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} FILES_MATCHING PATTERN memilio/*/*.h)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/memilio-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/memilio-config.cmake
INSTALL_DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/memilio
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/memilio-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install (
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/memilio-config-version.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/memilio-config.cmake"
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/memilio
)
