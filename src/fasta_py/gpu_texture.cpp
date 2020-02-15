#include "fasta_py/gpu_texture.h"


namespace fst { namespace py {

void export_GPU_Texture() {
	boost::python::class_<GPU_Texture, boost::noncopyable>("GPU_Texture", boost::python::no_init)
		.def("bind", &GPU_Texture::bind)
		.def("unbind", &GPU_Texture::unbind)
		;
}

void export_GPU_TextureManager() {
	boost::python::class_<GPU_TextureManager, boost::noncopyable>("GPU_TextureManager")
		.def("textureFromImage2D", &GPU_TextureManager::textureFromImage2D, boost::python::return_value_policy<boost::python::reference_existing_object>())
		;

	boost::python::enum_<TexFlags>("TexFlags")
    .value("COMPRESS", TexFlags::COMPRESS)
    .value("CACHE_TO_DISK", TexFlags::CACHE_TO_DISK)
    ;
}

}} // namespace