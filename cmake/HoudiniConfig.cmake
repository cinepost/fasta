# Package configuration file for the HDK.
#
# Defines the following imported library targets:
# - Houdini
#
# Defines the following variables:
# - Houdini_FOUND
# - Houdini_VERSION (from HoudiniConfigVersion.cmake)
# - Houdini_VERSION_MAJOR
# - Houdini_VERSION_MINOR
# - Houdini_VERSION_PATCH

include( CMakeParseArguments ) # For compatibility with CMake < 3.4.

set( _houdini_shared_libs dsolib/libHoudiniAPPS3.so;dsolib/libHoudiniAPPS2.so;dsolib/libHoudiniUI.so;dsolib/libHoudiniUT.so;dsolib/libhboost_system.so;dsolib/libHoudiniPRM.so;dsolib/libHoudiniHARD.so;dsolib/libHoudiniHAPIL.so;dsolib/libHoudiniOP2.so;dsolib/libHoudiniOP1.so;dsolib/libHoudiniSIM.so;dsolib/libHoudiniGEO.so;dsolib/libHoudiniPDG.so;dsolib/libHoudiniOPZ.so;dsolib/libHoudiniOP3.so;dsolib/libHoudiniOP4.so;dsolib/libHoudiniUSD.so;dsolib/libHoudiniAPPS1.so;dsolib/libHoudiniDEVICE.so )
set( _houdini_shared_lib_targets HoudiniAPPS3;HoudiniAPPS2;HoudiniUI;HoudiniUT;hboost_system;HoudiniPRM;HoudiniHARD;HoudiniHAPIL;HoudiniOP2;HoudiniOP1;HoudiniSIM;HoudiniGEO;HoudiniPDG;HoudiniOPZ;HoudiniOP3;HoudiniOP4;HoudiniUSD;HoudiniAPPS1;HoudiniDEVICE )
set( _houdini_static_libs  )
set( _houdini_static_lib_targets  )

set( _houdini_compile_options -fPIC;$<$<COMPILE_LANGUAGE:CXX>:-std=c++14>;-W;-Wall;-Wno-parentheses;-Wno-reorder;-Wno-sign-compare;-Wno-uninitialized;-Wno-unused-parameter;-Wunused;-Wdeprecated-declarations;-Wno-deprecated;-D_GLIBCXX_USE_CXX11_ABI=0;-fno-strict-aliasing;-Wno-unused-local-typedefs;$<$<NOT:$<VERSION_LESS:$<CXX_COMPILER_VERSION>,7.0>>:-faligned-new> )
set( _houdini_defines VERSION="18.0.348";AMD64;SIZEOF_VOID_P=8;FBX_ENABLED=1;OPENCL_ENABLED=1;OPENVDB_ENABLED=1;SESI_LITTLE_ENDIAN;$<$<CONFIG:Release>:UT_ASSERT_LEVEL=0>;$<$<CONFIG:RelWithDebInfo>:UT_ASSERT_LEVEL=1>;$<$<CONFIG:Debug>:UT_ASSERT_LEVEL=2>;_GNU_SOURCE;ENABLE_THREADS;USE_PTHREADS;_REENTRANT;_FILE_OFFSET_BITS=64;LINUX )

set( _houdini_release_version 18.0 )

if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    set( _houdini_platform_linux TRUE )
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    set( _houdini_platform_win TRUE )
elseif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
    set( _houdini_platform_osx TRUE )
endif ()

if ( _houdini_platform_osx )
    get_filename_component( _houdini_toolkit_realpath ${CMAKE_CURRENT_LIST_DIR} REALPATH )

    # Locate the root of the Houdini installation.
    set( _houdini_install_root "${_houdini_toolkit_realpath}/../../../../../../.." )
    get_filename_component( _houdini_install_root ${_houdini_install_root} REALPATH )

    # Locate the root of $HFS.
    set( _houdini_hfs_root "${CMAKE_CURRENT_LIST_DIR}/../.." )
    get_filename_component( _houdini_hfs_root ${_houdini_hfs_root} REALPATH )
else ()
    set( _houdini_install_root "${CMAKE_CURRENT_LIST_DIR}/../.." )
    get_filename_component( _houdini_install_root ${_houdini_install_root} ABSOLUTE )
    set( _houdini_hfs_root ${_houdini_install_root} )
endif ()

set( _houdini_include_dir "${_houdini_hfs_root}/toolkit/include" )
set( _houdini_bin_dir "${_houdini_hfs_root}/bin" )

# Add interface target.
add_library( Houdini INTERFACE )
target_include_directories( Houdini SYSTEM INTERFACE
	${_houdini_include_dir}
	${_houdini_include_dir}/python2.7
)
target_compile_options( Houdini INTERFACE ${_houdini_compile_options} )
target_compile_definitions( Houdini INTERFACE ${_houdini_defines} -DMAKING_DSO )

function( _houdini_create_libraries )
    cmake_parse_arguments( H_LIB
        ""
        "TYPE"
        "PATHS;TARGET_NAMES"
        ${ARGN}
    )

    list( LENGTH H_LIB_PATHS num_libs )
    if ( NOT num_libs )
        return ()
    endif ()
    math( EXPR num_libs "${num_libs} - 1" )
    foreach ( idx RANGE ${num_libs} )
        list( GET H_LIB_PATHS ${idx} lib_path )
        list( GET H_LIB_TARGET_NAMES ${idx} base_target_name )
        set( target_name Houdini::${base_target_name} )

        add_library( ${target_name} ${H_LIB_TYPE} IMPORTED )

        if ( ${H_LIB_TYPE} STREQUAL "STATIC" OR NOT _houdini_platform_win )
            set( import_property IMPORTED_LOCATION )
        else ()
            # IMPORTED_IMPLIB is used on Windows.
            set( import_property IMPORTED_IMPLIB )
        endif ()

        set_target_properties(
            ${target_name}
            PROPERTIES
                ${import_property} ${_houdini_install_root}/${lib_path}
        )

        target_link_libraries( Houdini INTERFACE ${target_name} )
    endforeach ()

endfunction ()

_houdini_create_libraries(
    PATHS ${_houdini_shared_libs}
    TARGET_NAMES ${_houdini_shared_lib_targets}
    TYPE SHARED
)

_houdini_create_libraries(
    PATHS ${_houdini_static_libs}
    TARGET_NAMES ${_houdini_static_lib_targets}
    TYPE STATIC
)

# Returns the default installation directory for the platform. For example, on
# Linux this is $HOME/houdiniX.Y.
# This can be used for installing additional files such as help or HDAs.

# Usage: houdini_get_default_install_dir( <VAR> )
function ( houdini_get_default_install_dir output_var )

    set( hython_path "${_houdini_bin_dir}/hython${CMAKE_EXECUTABLE_SUFFIX}" )

    # Run hython to retrieve the correct value of $HOUDINI_USER_PREF_DIR.
    execute_process(
        COMMAND ${hython_path} -c "from __future__ import print_function\nprint(hou.homeHoudiniDirectory())"
        OUTPUT_VARIABLE install_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE
	RESULT_VARIABLE status_code
	ERROR_VARIABLE status_str
    )

    if ( NOT status_code EQUAL 0 )
	message( FATAL_ERROR "Error running ${hython_path}: ${status_str}" )
    endif ()

    set( ${output_var} ${install_dir} PARENT_SCOPE )
endfunction ()

# - Sets the output directory for the target.
# - Sets the prefix for the library name.
#
# Usage: houdini_configure_target( target_name [INSTDIR dir] )
#
# Arguments:
# - INSTDIR: Output directory for the library / executable.
#            If not specified, the dso folder under the
#            houdini_get_default_install_dir() path (e.g. $HOME/houdiniX.Y/dso)
#            is used for libraries.
#            For executables, the default is ${CMAKE_CURRENT_BINARY_DIR}.
# - LIB_PREFIX: prefix for the library name. If not specified, the empty string
#               is used (e.g. "SOP_Star.so")
function ( houdini_configure_target target_name )
    cmake_parse_arguments( H_OUTPUT
        ""
        "INSTDIR;LIB_PREFIX"
        ""
        ${ARGN}
    )

    # Set the library prefix.
    set( prefix "" )
    if ( H_OUTPUT_LIB_PREFIX )
        set( prefix ${H_OUTPUT_LIB_PREFIX} )
    endif ()

    set_target_properties( ${target_name} PROPERTIES
        PREFIX "${prefix}"
    )

    # Configure the output directory for the library / executable.
    if ( NOT H_OUTPUT_INSTDIR )
        get_target_property( target_type ${target_name} TYPE )
        if ( ${target_type} STREQUAL "EXECUTABLE" )
            set( H_OUTPUT_INSTDIR ${CMAKE_CURRENT_BINARY_DIR} )
        else ()
            houdini_get_default_install_dir( H_OUTPUT_INSTDIR )
            string( APPEND H_OUTPUT_INSTDIR "/dso" )
        endif ()
    endif ()

    set( output_dir_properties
        LIBRARY_OUTPUT_DIRECTORY
        LIBRARY_OUTPUT_DIRECTORY_DEBUG
        LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO
        LIBRARY_OUTPUT_DIRECTORY_RELEASE
        RUNTIME_OUTPUT_DIRECTORY
        RUNTIME_OUTPUT_DIRECTORY_DEBUG
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
        RUNTIME_OUTPUT_DIRECTORY_RELEASE
        ARCHIVE_OUTPUT_DIRECTORY
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG
        ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE
    )
    foreach ( output_dir_property ${output_dir_properties} )
        set_target_properties( ${target_name}
            PROPERTIES ${output_dir_property} "${H_OUTPUT_INSTDIR}" )
    endforeach ()

endfunction ()


set( _houdini_python_version 2.7 )
set( _houdini_python_full_version 2.7.15-8 )
set( _houdini_python_dotless_version 27 )

if ( _houdini_platform_win )
    set( _python_binary "${_houdini_install_root}/python${_houdini_python_dotless_version}/python${_houdini_python_version}" )
elseif ( _houdini_platform_linux )
    set( _python_binary "${_houdini_install_root}/python/bin/python" )
elseif ( _houdini_platform_osx )
    set( _python_binary "${_houdini_install_root}/Frameworks/Python.framework/Versions/${_houdini_python_version}/bin/python" )
else ()
    message( FATAL_ERROR "Not implemented." )
endif ()

set( H_GEN_PROTO_SCRIPT ${_houdini_hfs_root}/houdini/python${_houdini_python_version}libs/generate_proto.py )

# Usage: houdini_generate_proto_headers(
#           OUTPUT_VAR generated_headers
#           FILES src.C...
#       )
#
#   - OUTPUT_VAR: Name of a variable to contain the list of generated headers.
#   - API: API name for the generated class (e.g. SOP_API)
#   - FILES: A list of .C files.
function ( houdini_generate_proto_headers )
    cmake_parse_arguments( H_PROTO "" "OUTPUT_VAR;API" "FILES" ${ARGN} )

    set( api_arg )
    if ( H_PROTO_API )
        set( api_arg "--api=${H_PROTO_API}" )
    endif ()

    set( generated_headers )
    foreach ( proto_file ${H_PROTO_FILES} )
        get_filename_component( fullname ${proto_file} NAME )
        string( REGEX REPLACE "\\.[^.]*$" "" basename ${fullname} )
        set( dst_header ${CMAKE_CURRENT_BINARY_DIR}/${basename}.proto.h )
        list( APPEND generated_headers ${dst_header} )

        add_custom_command(
            OUTPUT ${dst_header}
            COMMAND ${_python_binary} ${H_GEN_PROTO_SCRIPT} ${api_arg} ${proto_file} ${dst_header}
            DEPENDS ${proto_file} ${H_GEN_PROTO_SCRIPT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

        # Ensure the generated header is built before the corresponding .C file.
        set_source_files_properties(
            ${proto_file}
            PROPERTIES
            OBJECT_DEPENDS ${dst_header}
        )
    endforeach ()

    if ( H_PROTO_OUTPUT_VAR )
        set( ${H_PROTO_OUTPUT_VAR} ${generated_headers} PARENT_SCOPE )
    endif ()

endfunction ()
