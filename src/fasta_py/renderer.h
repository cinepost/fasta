#ifndef __FASTA_PY_RENDERER_H__
#define __FASTA_PY_RENDERER_H__

#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python/extract.hpp>

#include "fasta_lib/renderer.h"


namespace fst { namespace py {

using namespace fst;


void export_Renderer();

}}

#endif //__FASTA_PY_RENDERER_H__