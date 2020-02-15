#include "fasta_py/renderer.h"


namespace fst { namespace py {


void export_Renderer() {
	boost::python::class_<FstRenderer, boost::noncopyable>("Renderer")
		.def("init", &FstRenderer::init)
		;
}

}} // namespace