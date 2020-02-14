#ifndef __FASTA_PY_RENDERER_H__
#define __FASTA_PY_RENDERER_H__

#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python/extract.hpp>

#include "fasta_lib/renderer.h"


namespace fasta_module {

using namespace fst;

class Renderer {
	public:
		Renderer();
		~Renderer();

		bool init(uint width, uint height, uint samples);

	private:
		FstRenderer *_renderer;
};

void export_FstRenderer();

}

#endif //__FASTA_PY_RENDERER_H__