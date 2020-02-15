#ifndef __FASTA_GL_CONTEXT_H__
#define __FASTA_GL_CONTEXT_H__

#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/OpenGL.h>
	#include <OpenGL/CGLTypes.h>
	#include <OpenGL/CGLCurrent.h>
#elif __linux__
	#include <X11/X.h>
	#include <X11/Xlib.h> 
	#include <GL/gl.h>
	#include <GL/glx.h>
#endif

#include "fasta_lib/gl_info.h"

#ifdef __linux__
	typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
	typedef Bool (*glXMakeContextCurrentARBProc)(Display*, GLXDrawable, GLXDrawable, GLXContext);
	static glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;
	static glXMakeContextCurrentARBProc glXMakeContextCurrentARB = NULL;
#endif

namespace fst {

class GL_Context {
	public:
		GL_Context();
		~GL_Context();

		bool makeCurrent();
		bool init();

	private:
		bool _initialized;
		GL_Info	*_gl_info;

	private:
		#ifdef __APPLE__
			CGLContextObj	ctx;
		#elif __linux__
			GLXContext 		ctx;

			Display* 		dpy;
	    	GLXFBConfig* 	fbc;
	    	GLXPbuffer 		pbuf;
	    #endif

};

} // namsepace

#endif // __FASTA_GL_CONTEXT_H__