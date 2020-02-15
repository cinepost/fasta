#ifndef __FASTA_PY_GPU_TEXTURE_H__
#define __FASTA_PY_GPU_TEXTURE_H__

#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python/extract.hpp>

#include "fasta_lib/gpu_texture.h"
#include "fasta_lib/gpu_texture_manager.h"


namespace fst { namespace py {

using namespace fst;


void export_GPU_Texture();

void export_GPU_TextureManager();

}} // namespace

#endif //__FASTA_PY_GPU_TEXTURE_H__