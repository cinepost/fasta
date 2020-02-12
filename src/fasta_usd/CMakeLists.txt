set(BUILD_SHARED_LIBS ON)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/defaults
                      ${CMAKE_SOURCE_DIR}/cmake/modules
                      ${CMAKE_SOURCE_DIR}/cmake/macros
                      /opt/USD
)

include(pxrConfig)

include(ProjectDefaults)
include(Packages)

include(CXXDefaults)
add_definitions(${_PXR_CXX_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${_PXR_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")


include(Public)


FIND_PACKAGE(pxr CONFIG REQUIRED)

INCLUDE_DIRECTORIES(${PXR_INCLUDE_DIRS})
LINK_DIRECTORIES(${PXR_LIBRARIES})

add_subdirectory( hdFasta )

