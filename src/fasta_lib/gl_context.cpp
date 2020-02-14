#include <glad/glad.h>

#include "fasta_lib/gl_context.h"
#include "fasta_utils_lib/logging.h"

namespace fst {

FstGLContext::FstGLContext() {
	_initialized = false;	
}

FstGLContext::~FstGLContext() {
	LOG_DBG << "Fasta OpenGL context destructor called";
	if( _initialized ) {
		#ifdef __APPLE__
			CGLSetCurrentContext( NULL );
  			CGLDestroyContext( ctx );
		#elif __linux__

		#endif
	}
	LOG_DBG << "Fasta OpenGL context destructed";
}

#ifdef __APPLE__
bool FstGLContext::init(){
	if (_initialized)
		return true;

	LOG_DBG << "Fasta OpenGL context init on MacOS called";

	CGLPixelFormatAttribute attributes[4] = {
  		kCGLPFAAccelerated,   // no software rendering
  		kCGLPFAOpenGLProfile, // core profile with the version stated below
  		(CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
  		(CGLPixelFormatAttribute) 0
	};

	CGLPixelFormatObj pix;
	CGLError errorCode;
	GLint num; // stores the number of possible pixel formats
	errorCode = CGLChoosePixelFormat( attributes, &pix, &num );
	// add error checking here
	errorCode = CGLCreateContext( pix, NULL, &ctx ); // second parameter can be another context for object sharing
	// add error checking here
	CGLDestroyPixelFormat( pix );

	// try to make it the current context */
	if( !makeCurrent() ) {
		return false;
	}

	_initialized = true;
	return _initialized;

	LOG_DBG << "Fasta OpenGL context on MacOS initalized";
}
#elif __linux__
bool FstGLContext::init(){
	if (_initialized)
		return true;

	LOG_DBG << "Fasta OpenGL context init on Linux called";

	static int visual_attribs[] = {
		None
	};
	
	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 1,
		None
	};

	int fbcount = 0;
	fbc = NULL;

	// open display
    if ( ! (dpy = XOpenDisplay(NULL)) ){
        fprintf(stderr, "Failed to open display\n");
        return false;
    }

    // get framebuffer configs, any is usable (might want to add proper attribs)
    if ( !(fbc = glXChooseFBConfig(dpy, DefaultScreen(dpy), visual_attribs, &fbcount) ) ){
        fprintf(stderr, "Failed to get FBConfig\n");
        return false;
    }
 
    // get the required extensions
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB");
    glXMakeContextCurrentARB = (glXMakeContextCurrentARBProc)glXGetProcAddressARB( (const GLubyte *) "glXMakeContextCurrent");
    if ( !(glXCreateContextAttribsARB && glXMakeContextCurrentARB) ){
        fprintf(stderr, "missing support for GLX_ARB_create_context\n");
        XFree(fbc);
        return false;
    }
 
    // create a context using glXCreateContextAttribsARB
    if ( !( ctx = glXCreateContextAttribsARB(dpy, fbc[0], 0, True, context_attribs)) ){
        fprintf(stderr, "Failed to create opengl context\n");
        XFree(fbc);
        return false;
    }

	// create temporary pbuffer
	int pbuffer_attribs[] = {
		GLX_PBUFFER_WIDTH, 256,
		GLX_PBUFFER_HEIGHT, 256,
		None
	};

	pbuf = glXCreatePbuffer(dpy, fbc[0], pbuffer_attribs);
 
	XFree(fbc);
	XSync(dpy, False);
	
	// try to make it the current context */
	if( !makeCurrent() ) {
		return false;
	}
	

	int gladInitRes = gladLoadGL();
    if (!gladInitRes) {
      	fprintf(stderr, "Unable to initialize glad\n");
       	return false;
    }

    fprintf(stdout, "OpenGL vendor: %s\n", (const char*)glGetString(GL_VENDOR));

	// check for maximum render buffers
	int	maxDrawBuffers;
	glGetIntegerv ( GL_MAX_DRAW_BUFFERS_ARB, &maxDrawBuffers );
	LOG_DBG << "OpenGL maximum render buffers: " << maxDrawBuffers;

	LOG_DBG << "Fasta OpenGL context on Linux initalized";

	_initialized = true;
	return _initialized;
}
#endif

bool FstGLContext::makeCurrent() {
#ifdef __APPLE__
	CGLError errorCode;
	errorCode = CGLSetCurrentContext( ctx );
	// add error checking here

#elif __linux__
	if ( !glXMakeContextCurrent(dpy, pbuf, pbuf, ctx) ){
		// some drivers does not support context without default framebuffer, so fallback on using the default window.
		if ( !glXMakeContextCurrent(dpy, DefaultRootWindow(dpy), DefaultRootWindow(dpy), ctx) ){
			LOG_FTL << "Failed to make opengl context current";
			return false;
		}
	}
#endif
	return true;
}

} // namespace