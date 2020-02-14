#include <iostream>

#include "fasta_lib/gpu_texture.h"
#include "fasta_utils_lib/logging.h"

namespace fst {

GPU_Texture::GPU_Texture() {
	LOG_DBG << "GPU_Texture constructor called\n";
	
	LOG_DBG << "GPU_Texture constructed\n";
}

GPU_Texture::~GPU_Texture() {
	LOG_DBG << "GPU_Texture destructor called\n";

	LOG_DBG << "GPU_Texture destructed\n";
}

} // namespace