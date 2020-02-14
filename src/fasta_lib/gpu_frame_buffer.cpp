#include <iostream>

#include "fasta_lib/gpu_frame_buffer.h"
#include "fasta_utils_lib/logging.h"

namespace fst {

GPU_FrameBuffer::GPU_FrameBuffer() {
	LOG_DBG << "GPU_FrameBuffer constructor called\n";
	_is_init = false;
	_is_bound = false;
	_is_multisampled = false;
	_gl_object = 0;
	LOG_DBG << "GPU_FrameBuffer constructed\n";
}

GPU_FrameBuffer::~GPU_FrameBuffer() {
	LOG_DBG << "GPU_FrameBuffer destructor called\n";
	LOG_DBG << "GPU_FrameBuffer destructed\n";
}

bool GPU_FrameBuffer::init(uint width, uint height){
	LOG_DBG << "GPU_FrameBuffer "<< width << " x " << height <<" init called\n";
	
	_width = width;
	_height = height;

	glGenFramebuffers(1, &_gl_object);
	glBindFramebuffer(GL_FRAMEBUFFER, _gl_object);

	// - Position color buffer
	std::cout << "Fasta GBuffer position buffer ..." << std::endl;
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
	attachments.push_back(GL_COLOR_ATTACHMENT0);

	// - Normal color buffer
	std::cout << "Fasta GBuffer normal buffer ..." << std::endl;
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	attachments.push_back(GL_COLOR_ATTACHMENT1);

	// - Color + Metalness color buffer
	std::cout << "Fasta GBuffer albedo buffer ..." << std::endl;
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
	attachments.push_back(GL_COLOR_ATTACHMENT2);

	// - Depth
	std::cout << "Fasta GBuffer depth buffer ..." << std::endl;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);	

	/*
	// - Create and attach depth buffer (renderbuffer)
	GLuint rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	*/

	// - Finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		std::cerr << "Framebuffer not complete!" << std::endl;
	}

	glBindTexture( GL_TEXTURE_2D, 0 );
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	_is_init = true;
	return _is_init;
}

void GPU_FrameBuffer::resize(uint width, uint height) {
	
}

void GPU_FrameBuffer::bindForWriting(){
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _gl_object);
}

void GPU_FrameBuffer::bindForReading(){
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _gl_object);
}

void GPU_FrameBuffer::bind(){
	if (!_is_bound) {
		glBindFramebuffer(GL_FRAMEBUFFER, _gl_object);
		_is_bound = true;
	}

	// - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
	glDrawBuffers(3, &attachments[0]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, _width, _height);
}

void GPU_FrameBuffer::unbind(){
	if (_is_bound) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		_is_bound = false;
	}
}

void GPU_FrameBuffer::readData(uint data_type, void *data){
	bind();

	if( data_type == FST_COLOR_BUFFER ){
		glReadBuffer(GL_COLOR_ATTACHMENT2);
		glReadPixels(0, 0, _width, _height, GL_RGBA, GL_HALF_FLOAT, data);
	};

	if( data_type == FST_ALBEDO_BUFFER ){
		glReadBuffer(GL_COLOR_ATTACHMENT2);
		glReadPixels(0, 0, _width, _height, GL_RGBA, GL_HALF_FLOAT, data);
	}

	if( data_type == FST_NORMAL_BUFFER ){
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glReadPixels(0, 0, _width, _height, GL_RGB, GL_HALF_FLOAT, data);
	}

	if( data_type == FST_POSITION_BUFFER ){
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(0, 0, _width, _height, GL_RGB, GL_HALF_FLOAT, data);
	}

	unbind();
}

} // namespace