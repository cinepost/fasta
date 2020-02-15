#include <glad/glad.h>

#include "fasta_lib/gl_context.h"
#include "fasta_utils_lib/logging.h"

namespace fst {

GL_Context::GL_Context() {
	_initialized = false;
	_gl_info = nullptr;	
}

GL_Context::~GL_Context() {
	LOG_DBG << "Fasta OpenGL context destructor called";

	if( _gl_info ) 
		delete _gl_info;

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
bool GL_Context::init(){
	if (_initialized)
		return true;

	LOG_DBG << "Fasta OpenGL context init on MacOS called";

	CGLPixelFormatAttribute attributes[4] = {
  		kCGLPFAAccelerated,   // no software rendering
  		kCGLPFAOpenGLProfile, // core profile with the version stated below
  		(CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
  		(CGLPixelFormatAttribute) 0
	};

	CGLPixelFormatObj pix;
	CGLError errorCode;
	GLint num; // stores the number of possible pixel formats
	errorCode = CGLChoosePixelFormat( attributes, &pix, &num );
	// Add error checking here
	
	errorCode = CGLCreateContext( pix, NULL, &ctx ); // second parameter can be another context for object sharing
	// Add error checking here
	
	CGLDestroyPixelFormat( pix );

	// Try to make it the current context
	if( !makeCurrent() ) {
		return false;
	}

	// Try to load GLAD
	int gladInitRes = gladLoadGL();
    if (!gladInitRes) {
      	LOG_FTL << "Unable to load GLAD !";
       	return false;
    }

	// Try to load GL info
	_gl_info = new GL_Info();
	if (!_gl_info->load()) {
		LOG_WRN << "Unable to load OpenGL information.";
	}
	_gl_info->printInfo();

	_initialized = true;
	return _initialized;

	LOG_DBG << "Fasta OpenGL context on MacOS initalized";
}
#elif __linux__
bool GL_Context::init(){
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
        LOG_FTL << "Failed to open display";
        return false;
    }

    // get framebuffer configs, any is usable (might want to add proper attribs)
    if ( !(fbc = glXChooseFBConfig(dpy, DefaultScreen(dpy), visual_attribs, &fbcount) ) ){
        LOG_FTL << "Failed to get FBConfig";
        return false;
    }
 
    // get the required extensions
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB");
    glXMakeContextCurrentARB = (glXMakeContextCurrentARBProc)glXGetProcAddressARB( (const GLubyte *) "glXMakeContextCurrent");
    if ( !(glXCreateContextAttribsARB && glXMakeContextCurrentARB) ){
        LOG_FTL << "Missing support for GLX_ARB_create_context";
        XFree(fbc);
        return false;
    }
 
    // create a context using glXCreateContextAttribsARB
    if ( !( ctx = glXCreateContextAttribsARB(dpy, fbc[0], 0, True, context_attribs)) ){
        LOG_FTL << "Failed to create opengl context";
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

	// Verify created context is a direct or indirect
  	if ( ! glXIsDirect ( dpy, ctx ) ) {
    	LOG_DBG << "Indirect GLX rendering context obtained";
  	} else {
    	LOG_DBG << "Direct GLX rendering context obtained";
  	}
	
	// Try to make it the current context
	if( !makeCurrent() ) {
		return false;
	}

	// Try to load GLAD
	int gladInitRes = gladLoadGL();
    if (!gladInitRes) {
      	LOG_FTL << "Unable to load GLAD !";
       	return false;
    }

	// Try to load GL info
	_gl_info = new GL_Info();
	if (!_gl_info->load()) {
		LOG_WRN << "Unable to load OpenGL information.";
	}
	_gl_info->printInfo();

	// check for maximum render buffers
	int	maxDrawBuffers;
	glGetIntegerv ( GL_MAX_DRAW_BUFFERS_ARB, &maxDrawBuffers );
	LOG_DBG << "OpenGL maximum render buffers: " << maxDrawBuffers;

	LOG_DBG << "Fasta OpenGL context on Linux initalized";

	_initialized = true;
	return _initialized;
}
#endif

bool GL_Context::makeCurrent() {
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