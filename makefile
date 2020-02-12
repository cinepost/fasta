# Installation directory.
INSTDIR = $(HOME)/houdini15.5
# List of C++ source files to build.
# SOP_Main.C registers the operators and handles the DSO-specifics.
SOURCES = \
    ./src/ROP_Xenon.cpp \
    ./src/xenon.cpp \
    ./src/XN_Render.cpp \
    ./src/XN_Display.cpp \
    ./src/XN_GBuffer.cpp \
    ./src/XN_Shader.cpp \
    ./src/XN_Mesh.cpp \
    ./src/XN_Ifd.cpp \
    ./src/XN_Object.cpp \
    ./src/XN_Light.cpp \
    ./src/XN_Reader.cpp \
    ./src/XN_Property.cpp \
    ./src/XN_Properties.cpp \
    ./src/XN_PropertiesManager.cpp
    #./Modifiers/SOP_Bend.C

# Icons for custom node operators.
ICONS = icons/ROP_Xenon.svg

# Use the highest optimization level.
OPTIMIZER = -O3

# Additional include directories.
INCDIRS = \
	-Isrc \
	-I/usr/local/include
    #-I/opt/studio/sdk/include \
    #-I/opt/studio/third_party/sdk/include

# Additional library directories.
LIBDIRS = \
	-L/usr/local/lib
    #-L/opt/studio/sdk/lib \
    #-L/opt/studio/third_party/sdk/lib

# Additional libraries.
LIBS = \
    -lGLEW
    #-lstudioSDK \
    #-lthird_partySDK

# Set the app name.
APPNAME = $(HOME)/dev/xenon/bin/xenon

# Include the GNU Makefile.
include $(HFS)/toolkit/makefiles/Makefile.gnu
