#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <string>
#include <iostream>

#include "fasta_py/fasta_module.h"
#include "fasta_py/renderer.h"
#include "fasta_py/gpu_texture.h"

namespace fst { namespace py {

void export_FstRenderer();

BOOST_PYTHON_MODULE(fasta){
  export_Renderer();
  export_GPU_Texture();
  export_GPU_TextureManager();
}

}}
