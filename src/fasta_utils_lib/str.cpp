#include "fasta_utils_lib/str.h"
#include "fasta_utils_lib/logging.h"

namespace fst { namespace ut { namespace str {

template<typename T>
inline std::string toString(const T &t) {
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

template<typename T>
inline T fromString( const std::string& s ) {
    std::istringstream stream( s );
    T t;
    stream >> t;
    return t;
}

}}} // namespace