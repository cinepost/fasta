#include <iostream>

#include "fasta_lib/renderer.h"
#include "fasta_utils_lib/logging.h"

namespace fst {

FstRenderer::FstRenderer()
{
	std::cout << "Fasta engine constructor called ..." << std::endl;
	renderbuffer = 0;
	initialized = false;
	current_time = 0;
	display = new FstDisplay();
    std::cout << "Fasta engine constructed ..." << std::endl;
}

FstRenderer::~FstRenderer(){
	std::cout << "Fasta engine destructor called ..." << std::endl;

	delete display;

	if(renderbuffer!=0) delete renderbuffer;

	if(initialized==true) {
		#ifdef __APPLE__
		CGLSetCurrentContext( NULL );
  		CGLDestroyContext( ctx );
		#elif __linux__

		#endif

	}
	std::cout << "Fasta engine destructor ..." << std::endl;
}

#ifdef __APPLE__
bool FstRenderer::init(){
	std::cout << "Fasta engine init on macos called ..." << std::endl;

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

	errorCode = CGLSetCurrentContext( ctx );
	// add error checking here

}
#elif __linux__
bool FstRenderer::init(){
	fprintf(stdout, "Fasta render init started\n");

	static int visual_attribs[] = {
		None
	};
	
	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 1,
		None
	};

	dpy = XOpenDisplay(NULL);
	fbcount = 0;
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
	if ( !glXMakeContextCurrent(dpy, pbuf, pbuf, ctx) ){
		// some drivers does not support context without default framebuffer, so fallback on using the default window.
		if ( !glXMakeContextCurrent(dpy, DefaultRootWindow(dpy), DefaultRootWindow(dpy), ctx) ){
			fprintf(stderr, "failed to make current\n");
			return false;
		}
	}
	
	int gladInitRes = gladLoadGL();
    if (!gladInitRes) {
      	fprintf(stderr, "Unable to initialize glad\n");
       	return false;
    }

    fprintf(stdout, "OpenGL vendor: %s\n", (const char*)glGetString(GL_VENDOR));

	// check for maximum render buffers
	glGetIntegerv ( GL_MAX_DRAW_BUFFERS_ARB, &maxDrawBuffers );
	std::cout << "OpenGL maximum render buffers: " << maxDrawBuffers << std::endl;

    // Setup and compile our shaders
    if (!shaderGeometryPass.load("g_buffer.vs", "g_buffer.frag")) {
    	return false;
    }
    
    if (!shaderLightingPass.load("deferred_shading.vs", "deferred_shading.frag")) {
    	return false;
    }

	std::cout << "Fasta engine initialised ..." << std::endl;

	initialized = true;
	return initialized;
}
#endif

void FstRenderer::_renderSample(uint sample, uint samples_total) {
	LOG_DBG << "rendering sample " << sample << " of " << samples_total;
}

FstGBuffer* FstRenderer::renderFrame(uint width, uint height, uint aa_samples){
	if(renderbuffer!=0){
		// re-use existing render buffer
		renderbuffer->resize(width, height); // it's ok since resize would do nothing if the size is not changing
	} else {
		renderbuffer = new FstGBuffer();
		if (!renderbuffer->init(width, height))
			return nullptr;
	}
	
	// Define the viewport dimensions
    glViewport(0, 0, width, height);

    // Setup some OpenGL options
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.55f, 0.55f, 0.55f, 0.33f);

    renderbuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear The Screen And The Depth Buffer

    // Set samplers
    shaderGeometryPass.use();
    glUniform1i(glGetUniformLocation(shaderLightingPass.getProgram(), "gAlbedoSpec"), 0);
    glUniform1i(glGetUniformLocation(shaderLightingPass.getProgram(), "gNormal"), 1);
    glUniform1i(glGetUniformLocation(shaderLightingPass.getProgram(), "gPosition"), 2);


	renderbuffer->unbind();
	return renderbuffer;
}

void FstRenderer::_renderBackground() {

}

bool FstRenderer::_renderTile(uint xl, uint xr, uint yb, uint yt, uint tx, uint ty){
	return true;
}

uint FstRenderer::getCompletedSamples() const {
	return 12;
}

FstObject* FstRenderer::newObject(){
	objects.push_back(new FstObject());
	return objects.back();
}

FstLight* FstRenderer::newLight(){
	lights.push_back(new FstLight());
	return lights.back();
}

FstDisplay* FstRenderer::getDisplay(){
	return display;
}

}