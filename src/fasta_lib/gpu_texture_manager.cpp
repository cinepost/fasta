#include <stdio.h>

#include "fasta_lib/gpu_texture_manager.h"
#include "fasta_utils_lib/logging.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace fst {

GPU_TextureManager::GPU_TextureManager() {
	LOG_DBG << "Fasta GPU_TextureManager constructor called";

	LOG_DBG << "Fasta GPU_TextureManager constructed";
}

GPU_TextureManager::~GPU_TextureManager() {

}

GPU_Texture *GPU_TextureManager::textureFromImage2D(const std::string &filename, TexFlags flags) {
	TexKey tex_key(filename, flags);

	// Retrun cached texture if present
	if (_gpu_textures.count(tex_key) > 0) {
		LOG_DBG << "GPU texture for \"" << filename << "\" already loaded";
		return _gpu_textures.at(tex_key);
	}

	int image_width, image_height, image_comps;
	unsigned char* image_data = stbi_load(filename.c_str(), &image_width, &image_height, &image_comps, STBI_rgb_alpha);
	if(!image_data) {
    	LOG_FTL << "Failed to load texture: " << filename << "!!!";
    	return nullptr;
	}

	GLuint _glo;

	glGenTextures(1, &_glo);
	if (!_glo) {
		LOG_FTL << "Cannot create GPU texture !!!";
		return nullptr;
	}

	GPU_Texture *gpu_tex = new GPU_Texture(_glo);

	gpu_tex->bind();

	// Set default texture scaling
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Prepare formats according to our source image  
	bool do_compression = flags & TexFlags::COMPRESS ? true: false;
   	GLenum format, internal_format;
   	if (image_comps == 3) {
    	format = internal_format = GL_RGB;
    	if (do_compression)
    		internal_format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
   	}
   	else if (image_comps == 4) {
    	format = internal_format = GL_RGBA;
    	if (do_compression)
    		internal_format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
   	}

   	// Load image data to GPU
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, image_data);

    if ( do_compression ) {
    	// Check texture was successfully compressed
    	GLint texture_compressed = 0;
    	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &texture_compressed);
    	if (texture_compressed != GL_TRUE) {
    		LOG_ERR << "Error compressing texture \"" << filename << "\" !";
    	}
    }

    // If we did texture compression, write(cache) the compressed image to disk
   	if ( do_compression && flags & TexFlags::CACHE_TO_DISK) {
   		GLint compressed_size = 0;
   		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressed_size);
   	
   		// Allocate a buffer to read back the compressed texture.
    	GLubyte *compressed_bytes = reinterpret_cast<GLubyte *>(malloc(sizeof(GLubyte) * compressed_size));

    	// Read back the compressed texture.
    	glGetCompressedTexImage(GL_TEXTURE_2D, 0, compressed_bytes);

    	LOG_DBG << "Read back " << compressed_size << " bytes of compressed texture data from GPU for \"" + filename + "\"";
    	LOG_DBG << "Uncompressed size " << image_width * image_height * image_comps << " bytes";

    	// Save the texture to a file.
    	_saveCompressedTexture(filename + ".fct", image_width, image_height, internal_format, compressed_size, compressed_bytes);

    	// Free our buffer.
    	free(compressed_bytes);
	}

	// Default texture edge handling (clamping).
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	gpu_tex->unbind();

	stbi_image_free(image_data);

	LOG_DBG << "Texture \"" << filename << "\" loaded to GPU" << (do_compression ? " with in-place compression" : "");

	_gpu_textures.insert(std::make_pair(tex_key, gpu_tex));
	return gpu_tex;
}

void GPU_TextureManager::_saveCompressedTexture(const std::string &filename, GLint width, GLint height,
	GLenum compressed_format, GLint size, GLubyte *pData) {
   	FILE *pFile = fopen(filename.c_str(), "wb");
   
   	if (!pFile)
      	return;

   	GLuint info[4];

   	info[0] = width;
   	info[1] = height;
   	info[2] = compressed_format;
   	info[3] = size;

   	fwrite(info, 4, 4, pFile);
   	fwrite(pData, size, 1, pFile);
   	fclose(pFile);
   	LOG_DBG << "Compressed GPU texture ( " << size << " bytes) saved to \"" << filename << "\"";
}

GLubyte *GPU_TextureManager::_loadCompressedTexture(const std::string &filename, GLint *width, GLint *height, 
	GLenum *compressed_format, GLint *size) {
   	FILE *pFile = fopen(filename.c_str(), "rb");
   	
   	if (!pFile)
      	return nullptr;

   	GLuint info[4];

   	fread(info, 4, 4, pFile);
   	*width = info[0];
   	*height = info[1];
   	*compressed_format = info[2];
   	*size = info[3];

	GLubyte *pData = reinterpret_cast<GLubyte *>(malloc(*size));
   	fread(pData, *size, 1, pFile);
   	fclose(pFile);
   	return pData;
   // Free pData when done...
}

void GPU_TextureManager::printStats() {

}

} // namespace

