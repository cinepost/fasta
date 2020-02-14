#ifndef __GPU_FRAME_BUFFER_H__
#define __GPU_FRAME_BUFFER_H__

#include <vector>

#include <glad/glad.h>

#define FST_COLOR_BUFFER 0
#define FST_ALBEDO_BUFFER 1
#define FST_NORMAL_BUFFER 2
#define FST_POSITION_BUFFER 3
#define FST_DEPTH_BUFFER 4

typedef unsigned int uint;

namespace fst {

class GPU_FrameBuffer {
public:
	GPU_FrameBuffer();
	~GPU_FrameBuffer();

	bool	init(uint width, uint height);

	void	bind();
	void	unbind();
	void	bindForWriting();
	void	bindForReading();

	void	resize(uint width, uint height);
	void 	readData(uint data_type, void *data);

private:
	bool 	_is_init;
	bool 	_is_bound;
	bool	_is_multisampled;
	uint 	_width, _height;

	GLuint 	_gl_object;
	GLuint 	depthTexture, gPosition, gNormal, gAlbedoSpec;

	std::vector<GLenum> attachments;
};

} // namespace

#endif // __GPU_FRAME_BUFFER_H__