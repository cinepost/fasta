file( GLOB SOURCES
    ./*.cpp
)

# FindOpenGL
set( OpenGL_GL_PREFERENCE "GLVND" )
find_package(OpenGL REQUIRED)
if (OpenGL_FOUND)
    include_directories(${OPENGL_INCLUDE_DIRS})
    link_libraries(${OPENGL_LIBRARIES})
endif()

# find boost libs
find_package( Boost COMPONENTS log log_setup thread filesystem system REQUIRED )
include_directories( ${Boost_INCLUDE_DIRS} )

# we will use static libs
add_definitions(-DBOOST_LOG_DYN_LINK=1)
#set_target_properties(fasta_utils_lib PROPERTIES COMPILE_DEFINITIONS "BOOST_LOG_DYN_LINK")

add_library( fasta_utils_lib SHARED ${SOURCES} ${HEADERS} )

target_link_libraries(
    fasta_utils_lib
    ${Boost_LOG_SETUP_LIBRARY}
    ${Boost_LOG_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
)

if(APPLE)
    if( COPPER_BUILD_LIBS_AS_FRAMEWORKS )
	copper_config_framework( fasta_utils_lib )

    # Framework is called 'Toolbox'
    set_target_properties( fasta_utils_lib PROPERTIES OUTPUT_NAME "FastaUtils" )
    install( TARGETS fasta_utils_lib DESTINATION "Frameworks" )
  else()
    install( TARGETS fasta_utils_lib DESTINATION "lib" )
  endif()

elseif(UNIX)
    install( TARGETS fasta_utils_lib DESTINATION "lib" )
endif()
