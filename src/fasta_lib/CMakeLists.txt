file( GLOB SOURCES
    ./*.cpp
    ${PROJECT_SOURCE_DIR}/third_party/glad/src/glad.c
)

# FindOpenGL
set( OpenGL_GL_PREFERENCE "GLVND" )
find_package(OpenGL REQUIRED)
if (OpenGL_FOUND)
    include_directories(${OPENGL_INCLUDE_DIRS})
    link_libraries(${OPENGL_LIBRARIES})
endif()

# find boost libs
find_package( Boost COMPONENTS log REQUIRED )
include_directories( ${Boost_INCLUDE_DIRS} )

# we will use static libs
add_definitions(-DBOOST_LOG_DYN_LINK)

add_library( fasta_lib SHARED ${SOURCES} ${HEADERS} )

target_link_libraries(
    fasta_lib
    fasta_utils_lib
    ${OpenCL_LIBRARY} 
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
