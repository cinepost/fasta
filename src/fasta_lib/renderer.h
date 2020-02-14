#ifndef __FASTA_RENDERER_H__
#define __FASTA_RENDERER_H__

#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>

#include "fasta_lib/gl_context.h"
#include "fasta_lib/gpu_frame_buffer.h"
#include "fasta_lib/shader.h"
#include "fasta_lib/object.h"
#include "fasta_lib/light.h"
#include "fasta_lib/display.h"

namespace fst {

class FstRendererIPR;

class FstRenderer {
	friend class FstRendererIPR;

	public:
		FstRenderer();
		~FstRenderer();
		bool init(uint width, uint height, uint aa_samples);

		GPU_FrameBuffer *renderFrame();
		
		FstDisplay	*getDisplay();

		FstObject	*newObject();
		FstLight	*newLight();

		uint getCompletedSamples() const;

	private:
		bool _init_GL();

		void _renderSample();
		bool _renderTile(uint xl, uint xr, uint yb, uint yt, uint tx, uint ty);

		void _renderBackground();


		GPU_FrameBuffer *renderbuffer;
		FstDisplay *display;

	private:
		bool _render_initialized;
		bool _opengl_initialized;

		FstGLContext 	*ctx;

	private:
	    FstShader shaderGeometryPass, shaderLightingPass;

	private:
		float					current_time;
		std::vector<FstObject*>	objects;
		std::vector<FstLight*>	lights;

	private:
		uint _width;
		uint _height;
		uint _aa_samples;
		uint _sample_num;

};

}

#endif // __FASTA_RENDERER_H__