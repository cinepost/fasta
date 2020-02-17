#ifndef __FASTA_UTILS_VFS_H__
#define __FASTA_UTILS_VFS_H__

#include <ctime>
#include <cstring>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

namespace fst { namespace ut { namespace vfs {

std::time_t fileModificationTime(const std::string &filename);
bool fileExist(const std::string &filename);

}}} // namespace

#endif // __FASTA_UTILS_VFS_H__