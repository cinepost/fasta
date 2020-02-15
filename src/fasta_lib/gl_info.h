#ifndef __FASTA_GL_INFO_H__
#define __FASTA_GL_INFO_H__

#include <vector>
#include <string>

namespace fst {

class GL_Info {

	public:
		GL_Info();
		~GL_Info(){};

		bool load();
		void printInfo();
		uint versionCode() const;

	private:
		std::string _vendor;
    	std::string _renderer;
    	std::string _version;
    	//std::vector<std::string> _extensions;

    	uint _version_code;

    private:
    	bool	_gl_info_loaded;

};

} // namespace

#endif // __FASTA_GL_INFO_H__