#include <iostream>

#include "fasta_lib/gpu_texture.h"
#include "fasta_utils_lib/logging.h"

namespace fst {

GPU_Texture::GPU_Texture(const GLuint _glo) {
	LOG_DBG << "GPU_Texture constructor called\n";
	this->_glo = _glo;
	LOG_DBG << "GPU_Texture constructed\n";
}

GPU_Texture::~GPU_Texture() {
	LOG_DBG << "GPU_Texture destructor called\n";

	LOG_DBG << "GPU_Texture destructed\n";
}

void GPU_Texture::bind() {
	glBindTexture(GL_TEXTURE_2D, _glo);
}

void GPU_Texture::unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace