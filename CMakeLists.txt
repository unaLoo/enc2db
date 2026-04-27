cmake_minimum_required(VERSION 3.15)
project(enc2db LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(PYBIND11_NEWPYTHON ON)

install(DIRECTORY resource/ DESTINATION enc2db/resource)

find_package(pybind11 REQUIRED)

find_package(GDAL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(GEOS REQUIRED)
find_library(SPATIALITE_LIBRARY NAMES spatialite REQUIRED)

# 依赖头文件
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ISO8211
    ${CMAKE_CURRENT_SOURCE_DIR}/src/S57Parser
	${GDAL_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
    ${GEOS_INCLUDE_DIRS}
)

# 源文件
set(SOURCE_FILES
    src/binding.cpp
    src/DBHandler_Spatialite.cpp
    src/DBH_Base.cpp

    # ISO8211
    src/ISO8211/8211dump.cpp
    src/ISO8211/8211view.cpp
    src/ISO8211/ddffield.cpp
    src/ISO8211/ddffielddefn.cpp
    src/ISO8211/ddfmodule.cpp
    src/ISO8211/ddfrecord.cpp
    src/ISO8211/ddfsubfielddefn.cpp
    src/ISO8211/ddfutils.cpp
    src/ISO8211/mkcatalog.cpp
    src/ISO8211/timetest.cpp

    # S57Parser
    src/S57Parser/areafeature.cpp
    src/S57Parser/connectednodevector.cpp
    src/S57Parser/dsid.cpp
    src/S57Parser/dspm.cpp
    src/S57Parser/edgevector.cpp
    src/S57Parser/feature.cpp
    src/S57Parser/isolatednodevector.cpp
    src/S57Parser/linefeature.cpp
    src/S57Parser/MyChartLoader2.cpp
    src/S57Parser/pointfeature.cpp
    src/S57Parser/util.cpp
)

# pybind11_add_module(_enc2db_core ${SOURCE_FILES})
pybind11_add_module(_enc2db_core STABLE_ABI ${SOURCE_FILES})

target_link_libraries(_enc2db_core PRIVATE
    ${GDAL_LIBRARIES}
    ${SQLite3_LIBRARIES}
    ${SPATIALITE_LIBRARY}
    ${GEOS_LIBRARIES}
)


install(TARGETS _enc2db_core DESTINATION enc2db)