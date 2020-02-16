#ifndef __FASTA_UTILS_STR_H__
#define __FASTA_UTILS_STR_H__

#include <string>
#include <sstream>

namespace fst { namespace ut { namespace str {

template<typename T>
std::string toString(const T &t);

template<typename T>
T fromString(const std::string& s);

}}} // namespace

#endif // __FASTA_UTILS_STR_H__