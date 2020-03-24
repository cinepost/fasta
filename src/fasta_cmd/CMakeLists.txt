set (SOURCES 
	./fasta.cpp
	${PROJECT_SOURCE_DIR}/third_party/glad/src/glad.c
	${PROJECT_SOURCE_DIR}/third_party/imgui/imgui.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_draw.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_widgets.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui/examples/imgui_impl_glfw.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui/examples/imgui_impl_vulkan.cpp
)

# ImGui
include_directories(${PROJECT_SOURCE_DIR}/third_party/imgui)

# Find GLFW
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})
link_libraries(${GLFW_LIBRARIES})

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

# Find BOOST
find_package( Boost COMPONENTS program_options log REQUIRED )
include_directories( ${Boost_INCLUDE_DIRS} )
# we will use static libs
add_definitions(-DBOOST_LOG_DYN_LINK)

# RPATH 
SET(CMAKE_SKIP_BUILD_RPATH  FALSE) # use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) # when building, don't use the install RPATH already (but later on when installing)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) # add the automatically determined parts of the RPATH which point to directories outside the build tree to the install RPATH

# Executable 
add_executable ( fasta_cmd ${SOURCES} )
set_target_properties( fasta_cmd PROPERTIES OUTPUT_NAME "fasta" )
 
target_link_libraries( fasta_cmd
	fasta_lib
	fasta_utils_lib 
	Boost::log 
	Boost::program_options 
	${Vulkan_LIBRARY}
)

install(TARGETS fasta_cmd DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)