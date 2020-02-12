#include "fasta_py/renderer.h"


namespace fasta_module {

Renderer::Renderer() {
	_renderer = new FstRenderer();
}

Renderer::~Renderer() {
	delete _renderer;
}

bool Renderer::init() {
    return _renderer->init();
}

void export_FstRenderer() {
	boost::python::class_<Renderer, boost::noncopyable>("FstRenderer")
		.def("init", &Renderer::init)
		;
}

} // namespace