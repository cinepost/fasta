#include "fasta_py/renderer.h"


namespace fst { namespace py {


void export_Renderer() {
	boost::python::class_<FstRenderer, boost::noncopyable>("Renderer")
		.def("init", &FstRenderer::init)
		.def("textureManager", &FstRenderer::textureManager,
			boost::python::return_value_policy<boost::python::reference_existing_object>())
		;
}

}} // namespace