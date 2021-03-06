file( GLOB SOURCES
    ./*.cpp
    ./base/VulkanTools.cpp
    ${PROJECT_SOURCE_DIR}/third_party/glad/src/glad.c
)

set( CMAKE_MODULE_PATH 
    ${CMAKE_SOURCE_DIR}/cmake/modules
)

# Find OpenGL
set( OpenGL_GL_PREFERENCE "GLVND" )
find_package(OpenGL REQUIRED)
if (OpenGL_FOUND)
    include_directories(${OPENGL_INCLUDE_DIRS})
    link_libraries(${OPENGL_LIBRARIES})
endif()

# Use FindVulkan module added with CMAKE 3.7
if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
    message(STATUS "Using module to find Vulkan")
    find_package(Vulkan)
endif()

if (NOT Vulkan_FOUND)
    find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR/libs/vulkan}" REQUIRED)
    if (Vulkan_LIBRARY)
        set(Vulkan_FOUND OK)
        message("Using bundled vulkan library version")
    endif()
endif()

# Find Boost libs
find_package( Boost COMPONENTS log REQUIRED )
include_directories( ${Boost_INCLUDE_DIRS} )

# We use Boost static libs
add_definitions(-DBOOST_LOG_DYN_LINK)

add_library( fasta_lib SHARED ${SOURCES} ${HEADERS} )

# Find OpenImageIO
find_package( OpenImageIO REQUIRED)
include_directories(${OIIO_INCLUDE_DIRS})
link_libraries(${OIIO_LIBRARIES})

target_link_libraries(
    fasta_lib
    fasta_utils_lib
    ${OpenCL_LIBRARY}
    ${Vulkan_LIBRARY} 
    dl Boost::log
)

if(APPLE)
    if( COPPER_BUILD_LIBS_AS_FRAMEWORKS )
	copper_config_framework( fasta_lib )

    # Framework is called 'Fasta'
    set_target_properties( fasta_lib PROPERTIES OUTPUT_NAME "Fasta" )
    install( TARGETS fasta_lib DESTINATION "Frameworks" )
  else()
    install( TARGETS fasta_lib DESTINATION "lib" )
  endif()

elseif(UNIX)
    install( TARGETS fasta_lib DESTINATION "lib" )
endif()
