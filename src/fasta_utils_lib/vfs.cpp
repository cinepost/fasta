#include "fasta_utils_lib/vfs.h"
#include "fasta_utils_lib/logging.h"

namespace fst { namespace ut { namespace vfs {

std::time_t fileModificationTime(const std::string &filename) {
	boost::filesystem::path filepath(filename);
	if ( boost::filesystem::exists( filepath ) ) {
    	return boost::filesystem::last_write_time(filepath);
	}

	LOG_ERR << "Could not find file: " << filename; 
	return 0;
}

bool fileExist(const std::string &filename) {
	//boost::filesystem::path filepath(filename);
	return boost::filesystem::exists( filename );
}

}}} // namespace