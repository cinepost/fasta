#ifndef __FASTA_GBUFFER_H__
#define __FASTA_GBUFFER_H__

#include <vector>

#include <glad/glad.h>

#define FST_COLOR_BUFFER 0
#define FST_ALBEDO_BUFFER 1
#define FST_NORMAL_BUFFER 2
#define FST_POSITION_BUFFER 3
#define FST_DEPTH_BUFFER 4

namespace fst {

typedef unsigned int uint;

class FstGBuffer {
public:
	FstGBuffer();
	~FstGBuffer();

	bool	init(uint width, uint height);

	void	bind();
	void	unbind();
	void	bindForWriting();
	void	bindForReading();

	void	resize(uint width, uint height);
	void 	readData(uint data_type, void *data);

private:
	bool 	initialized;
	uint 	_width, _height;

	GLuint 	gBuffer;
	GLuint 	depthTexture, gPosition, gNormal, gAlbedoSpec;

	std::vector<GLenum> attachments;
};

} // namespace

#endif // __FASTA_GBUFFER_H__