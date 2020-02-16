#ifndef __GPU_TEXTURE_MANAGER_H__
#define __GPU_TEXTURE_MANAGER_H__

#include <utility>
#include <vector>
#include <string>
#include <map>

#include <glad/glad.h>
#include <flags/flags.hpp>

#include "fasta_lib/gpu_texture.h"

namespace fst {

enum TexFlags { 
	COMPRESS = 1 << 0, 
	CACHE_TO_DISK = 1 << 1 
};

typedef std::pair<std::string, TexFlags> TexKey;

class GPU_TextureManager {
	public:
		GPU_TextureManager();
		~GPU_TextureManager();

	public:
		GPU_Texture *textureFromImage2D(const std::string &filename, TexFlags flags);
		void printStats();

	private:
		void _saveCompressedTexture(const std::string &filename, GLint width, GLint height,
			GLenum compressedFormat, GLint size, GLubyte *pData);
		
		GLubyte *_loadCompressedTexture(const std::string &filename, GLint *width, GLint *height, 
			GLenum *compressed_format, GLint *size);

	private:
		std::map<TexKey, GPU_Texture*> 	_gpu_textures;

	private:
		// stats
		uint _cpu_mem_used; // bytes
		uint _gpu_mem_used; // bytes

};

} // namespace

ALLOW_FLAGS_FOR_ENUM(fst::TexFlags)

#endif // __GPU_TEXTURE_MANAGER_H__