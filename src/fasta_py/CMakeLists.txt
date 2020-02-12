file( GLOB SOURCES
	./*.cpp
)

# find python libs
find_package( PythonLibs 3 REQUIRED )
include_directories( ${PYTHON_INCLUDE_DIRS} )

# find boost libs
if(APPLE)
 find_package( Boost COMPONENTS python37 REQUIRED )
else()
 find_package( Boost COMPONENTS python3 REQUIRED )
endif()

include_directories( ${Boost_INCLUDE_DIRS} )

SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREADED ON)
SET(Boost_USE_STATIC_RUNTIME OFF)

find_package( OpenCL REQUIRED )
include_directories( ${OpenCL_INCLUDE_DIRS} )
link_directories( ${OpenCL_LIBRARY} )

# RPATH 
SET(CMAKE_SKIP_BUILD_RPATH  FALSE) # use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) # when building, don't use the install RPATH already (but later on when installing)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) # add the automatically determined parts of the RPATH which point to directories outside the build tree to the install RPATH


# define wrapper library
add_library( fasta SHARED ${SOURCES} )
target_include_directories ( fasta PUBLIC ${COPPER_INCLUDE_DIRECTORY} )

target_link_libraries( fasta ${Boost_LIBRARIES} ${PYTHON_LIBRARIES} ${OpenCL_LIBRARY} fasta_lib )

# don't prepend wrapper library name with lib
set_target_properties( fasta PROPERTIES PREFIX "" )
if (APPLE)
    set_target_properties( fasta PROPERTIES SUFFIX ".so" )
endif (APPLE)

install(TARGETS fasta DESTINATION lib/python3.7libs)
