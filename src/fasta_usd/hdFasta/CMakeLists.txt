set(PXR_PREFIX pxr/imaging)
set(PXR_PACKAGE hdFasta)
add_custom_target(shared_libs)

#if (NOT ${PXR_ENABLE_GL_SUPPORT})
#    message(STATUS
#        "Skipping ${PXR_PACKAGE} because PXR_ENABLE_GL_SUPPORT is OFF")
#    return()
#endif()

pxr_plugin(hdFasta
    PREFIX
        huy

    LIBRARIES
        plug
        tf
        vt
        gf
        work
        hf
        hd
        hdx
        xenon_lib

    INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/src

    PRIVATE_CLASSES
        #config
        #instancer
        #mesh
        #meshSamplers
        #renderBuffer
        
        #renderer
        rendererPlugin
        renderDelegate
        
        #renderPass
        #sampler

    #PUBLIC_HEADERS
    #    context.h
    #    renderParam.h

    RESOURCE_FILES
        plugInfo.json
)

#if (X11_FOUND OR APPLE)
#pxr_build_test(testhdFasta
#    LIBRARIES
#        hdFasta
#        garch
#        arch
#    CPPFILES
#        testenv/testhdFasta.cpp
#)
#endif()
