#include "fasta_py/renderer.h"


namespace fasta_module {

Renderer::Renderer() {
	_renderer = new FstRenderer();
}

Renderer::~Renderer() {
	delete _renderer;
}

bool Renderer::init(uint width, uint height, uint samples) {
    return _renderer->init(width, height, samples);
}

void export_FstRenderer() {
	boost::python::class_<Renderer, boost::noncopyable>("FstRenderer")
		.def("init", &Renderer::init)
		;
}

} // namespace