add_library(seir
    seir.h
    seir.cpp
)
target_link_libraries(seir PUBLIC memilio)
target_include_directories(seir PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_options(seir PRIVATE ${MEMILIO_CXX_FLAGS_ENABLE_WARNING_ERRORS})