#include "fasta_utils_lib/vfs.h"
#include "fasta_utils_lib/logging.h"

namespace fst { namespace ut { namespace vfs {

inline std::time_t fileModificationTime(const std::string &filename) {
	boost::filesystem::path filepath(filename);
	if ( boost::filesystem::exists( filepath ) ) {
    	return boost::filesystem::last_write_time(filepath);
	}

	LOG_ERR << "Could not find file: " << filename; 
	return 0;
}

}}} // namespace