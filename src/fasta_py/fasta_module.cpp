#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <string>
#include <iostream>

#include "fasta_py/fasta_module.h"
#include "fasta_py/renderer.h"

namespace fasta_module {

void export_FstRenderer();

BOOST_PYTHON_MODULE(fasta){
  export_FstRenderer();
}

}
