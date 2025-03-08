cmake_minimum_required(VERSION 3.4)
project(memilio-python)

option(MEMILIO_USE_BUNDLED_PYBIND11 "Use pybind11 bundled with this library." ON)
mark_as_advanced(MEMILIO_USE_BUNDLED_PYBIND11)

if(MEMILIO_USE_BUNDLED_PYBIND11)
  # Fetch pybind11
  include(FetchContent)
  FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11
    GIT_TAG v2.5
  )
  FetchContent_GetProperties(pybind11)
  if(NOT pybind11_POPULATED)
    FetchContent_Populate(pybind11)
    add_subdirectory(${pybind11_SOURCE_DIR} ${pybind11_BINARY_DIR})
  endif()
else()
    find_package(pybind11 REQUIRED)
endif()

# add in C++ library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../cpp ${CMAKE_CURRENT_BINARY_DIR}/cpp EXCLUDE_FROM_ALL)

# build python extensions
pybind11_add_module(_simulation_secir MODULE
    memilio/simulation/secir.cpp
)
target_link_libraries(_simulation_secir PRIVATE memilio secir)
target_include_directories(_simulation_secir PRIVATE memilio/simulation)
install(TARGETS _simulation_secir LIBRARY DESTINATION memilio)

pybind11_add_module(_simulation MODULE
    memilio/simulation/simulation.cpp
)
target_link_libraries(_simulation PRIVATE memilio)
target_include_directories(_simulation PRIVATE memilio/simulation)
install(TARGETS _simulation LIBRARY DESTINATION memilio)
