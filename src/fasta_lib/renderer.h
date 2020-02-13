#ifndef __FASTA_RENDERER_H__
#define __FASTA_RENDERER_H__

#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>

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

#include "fasta_lib/gbuffer.h"
#include "fasta_lib/shader.h"
#include "fasta_lib/object.h"
#include "fasta_lib/light.h"
#include "fasta_lib/display.h"

#ifdef __linux__
	typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
	typedef Bool (*glXMakeContextCurrentARBProc)(Display*, GLXDrawable, GLXDrawable, GLXContext);
	static glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;
	static glXMakeContextCurrentARBProc glXMakeContextCurrentARB = NULL;
#endif

namespace fst {

class FstRendererIPR;

class FstRenderer {
	friend class FstRendererIPR;

	public:
		FstRenderer();
		~FstRenderer();
		bool init();

		FstGBuffer *renderFrame(uint width, uint height);
		
		FstDisplay	*getDisplay();

		FstObject	*newObject();
		FstLight	*newLight();

		uint getCompletedSamples() const;

	private:
		void _renderSample(uint sample, uint samples_total);
		bool _renderTile(uint xl, uint xr, uint yb, uint yt, uint tx, uint ty);


		FstGBuffer *renderbuffer;
		FstDisplay *display;

	private:
		bool initialized;

		#ifdef __APPLE__
			CGLContextObj	ctx;
		#elif __linux__
			GLXContext 		ctx;

			Display* 		dpy;
	    	GLXFBConfig* 	fbc;
	    	GLXPbuffer 		pbuf;
	    #endif

	    int fbcount;
	    int	maxDrawBuffers;

	private:
	    FstShader shaderGeometryPass, shaderLightingPass;

	private:
		float					current_time;
		std::vector<FstObject*>	objects;
		std::vector<FstLight*>	lights;

	private:
		uint _width;
		uint _height;

};

}

#endif // __FASTA_RENDERER_H__