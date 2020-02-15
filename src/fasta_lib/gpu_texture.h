#ifndef __GPU_TEXTURE_H__
#define __GPU_TEXTURE_H__

#include <vector>

#include <glad/glad.h>

namespace fst {

class GPU_TextureManager;

struct TextureInfo {
	uint width;
	uint height;
	uint comps;

	uint mem_used; // bytes
};

class GPU_Texture {
	friend class GPU_TextureManager; //only GPU_TextureManager can create instances of GPU_Texture
	
	public:
		void bind();
		void unbind();

	private:
		GPU_Texture(const GLuint _glo);
		~GPU_Texture();

	private:
		TextureInfo _srctex_info;
		TextureInfo _gputex_info;

	private:
		GLuint _glo;
};

} // namespace

#endif // __GPU_TEXTURE_H__