#include <iostream>

#include "fasta_lib/renderer.h"
#include "fasta_utils_lib/logging.h"

namespace fst {

FstRenderer::FstRenderer() {
	LOG_DBG << "Fasta engine constructor called\n";
	renderbuffer = nullptr;
	_render_initialized = false;
	_opengl_initialized = false;
	current_time = 0;
	ctx = new FstGLContext();
	display = new FstDisplay();
    LOG_DBG << "Fasta engine constructed\n";
}

FstRenderer::~FstRenderer(){
	LOG_DBG << "Fasta engine destructor called\n";

	delete display;

	if(renderbuffer!=0) delete renderbuffer;

	delete ctx;

	LOG_DBG << "Fasta engine destructed\n";
}

/*
    // Setup and compile our shaders
    if (!shaderGeometryPass.load("g_buffer.vs", "g_buffer.frag")) {
    	return false;
    }
    
    if (!shaderLightingPass.load("deferred_shading.vs", "deferred_shading.frag")) {
    	return false;
    }
*/

bool FstRenderer::init(uint width, uint height, uint aa_samples){
	// chack if GL is initialized
	if(!ctx->init()){
		LOG_FTL << "Unable to initialize OpenGL !!!\n";
		return false;
	}

	_sample_num = 0;

	// check if the same init requested
	if (_render_initialized && _width==width && _height==height && _aa_samples==aa_samples ){
		return _render_initialized;
	}

	_width = width;
	_height = height;
	_aa_samples = aa_samples;
	_render_initialized = true;

	if(renderbuffer!=nullptr){
		// re-use existing render buffer
		renderbuffer->resize(width, height); // it's ok since resize would do nothing if the size is not changing
	} else {
		renderbuffer = new GPU_FrameBuffer();
		if (!renderbuffer->init(width, height))
			_render_initialized = false;
	}

	return _render_initialized;
}

void FstRenderer::_renderSample() {
	if (!_render_initialized) {
		LOG_ERR << "Fasta renderer not initalized! Abort sample rendering...\n";
	}

	LOG_DBG << "rendering sample " << _sample_num << " out of " << _aa_samples;
	_sample_num += 1;
}

GPU_FrameBuffer* FstRenderer::renderFrame(){
	while (_sample_num < _aa_samples) {
		_renderSample();
	}
	
	// Define the viewport dimensions
    glViewport(0, 0, _width, _height);

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