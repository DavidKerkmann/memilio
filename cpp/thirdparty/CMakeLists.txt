# Versions of the bundled libraries
# If you like to upgrade, just change the number
set(MEMILIO_EIGEN_VERSION "3.3.9")
set(MEMILIO_SPDLOG_VERSION "1.5.0") 
set(MEMILIO_JSONCPP_VERSION "1.7.4")

### SPDLOG
set(SPDLOG_INSTALL ON)
if(MEMILIO_USE_BUNDLED_SPDLOG)
    message(STATUS "Downloading Spdlog library")
    if(CMAKE_VERSION VERSION_LESS 3.11)
        set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")

        include(DownloadProject)
        download_project(PROJ                spdlog
                         GIT_REPOSITORY      https://github.com/gabime/spdlog.git
                         GIT_TAG             v${MEMILIO_SPDLOG_VERSION}
                         UPDATE_DISCONNECTED 1
                         QUIET
        )

        add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_SOURCE_DIR} EXCLUDE_FROM_ALL)
    else()
        include(FetchContent)
        FetchContent_Declare(
          spdlog
          GIT_REPOSITORY https://github.com/gabime/spdlog.git
          GIT_TAG v${MEMILIO_SPDLOG_VERSION}
        )
        FetchContent_GetProperties(spdlog)
        if(NOT spdlog_POPULATED)
          FetchContent_Populate(spdlog)
          add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR} EXCLUDE_FROM_ALL)
        endif()
    endif()
else()
    find_package(spdlog REQUIRED)
endif()

### EIGEN
if(MEMILIO_USE_BUNDLED_EIGEN)
    message(STATUS "Downloading Eigen library")
    if(CMAKE_VERSION VERSION_LESS 3.11)
        set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")

        include(DownloadProject)
        download_project(PROJ eigen
            GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
            GIT_TAG ${MEMILIO_EIGEN_VERSION}
            UPDATE_DISCONNECTED 1
            QUIET)
    else()
        include(FetchContent)
        FetchContent_Declare(eigen
            GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
            GIT_TAG ${MEMILIO_EIGEN_VERSION})
        FetchContent_GetProperties(eigen)
        if(NOT eigen_POPULATED)
          FetchContent_Populate(eigen)
        endif()
    endif()
    add_library(eigen INTERFACE)
    target_include_directories(eigen INTERFACE ${eigen_SOURCE_DIR})
    add_library(Eigen3::Eigen ALIAS eigen)
else()
    find_package(Eigen3 ${MEMILIO_EIGEN_VERSION} REQUIRED NO_MODULE)
endif()

### BOOST
if (MEMILIO_USE_BUNDLED_BOOST)
    include(BuildBoost)
else()
    find_package(Boost REQUIRED COMPONENTS outcome optional filesystem)
endif(MEMILIO_USE_BUNDLED_BOOST)

### HDF5
find_package(HDF5 COMPONENTS C)
if (HDF5_FOUND)
    set(MEMILIO_HAS_HDF5 ON)
else()
    message(WARNING "HDF5 was not found. Memilio will be built without some IO features. Install HDF5 Libraries and set the HDF5_DIR cmake variable to the directory containing the hdf5-config.cmake file to build with HDF5.")
endif()

### JSONCPP
if(MEMILIO_USE_BUNDLED_JSONCPP)
    message(STATUS "Downloading jsoncpp library")
    if(CMAKE_VERSION VERSION_LESS 3.11)
        set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")

        include(DownloadProject)
        download_project(PROJ               jsoncpp
                        URL                 https://github.com/open-source-parsers/jsoncpp/archive/${MEMILIO_JSONCPP_VERSION}.tar.gz
                        UPDATE_DISCONNECTED 1
                        QUIET
        )
        add_subdirectory(${jsoncpp_SOURCE_DIR} ${jsoncpp_SOURCE_DIR} EXCLUDE_FROM_ALL)
    else()
      include(FetchContent)
      FetchContent_Declare(
        jsoncpp
        URL https://github.com/open-source-parsers/jsoncpp/archive/${MEMILIO_JSONCPP_VERSION}.tar.gz
      )
      FetchContent_GetProperties(jsoncpp)
      if(NOT jsoncpp_POPULATED)
        FetchContent_Populate(jsoncpp)
        add_subdirectory(${jsoncpp_SOURCE_DIR} ${jsoncpp_BINARY_DIR} EXCLUDE_FROM_ALL)
      endif()
    endif()

    if (BUILD_SHARED_LIBS)
        add_library(JsonCpp::JsonCpp ALIAS jsoncpp_lib)
    else()
        add_library(JsonCpp::JsonCpp ALIAS jsoncpp_lib_static)
    endif()
else()
    find_package(jsoncpp CONFIG)
endif()

if (TARGET JsonCpp::JsonCpp)
    set(MEMILIO_HAS_JSONCPP ON)
else()
    message(WARNING "JsonCpp was not found. Memilio will be built without some IO features. 
        Set CMake variable MEMILIO_USE_BUNDLED_JSONCPP to ON or install JsonCpp and set the jsoncpp_DIR cmake variable 
        to the directory containing the jsoncppConfig.cmake file to build with JsonCpp.")
endif()
