#include <glad/glad.h>

#include "fasta_lib/gl_info.h"
#include "fasta_utils_lib/logging.h"

namespace fst {

GL_Info::GL_Info() { 
	_version_code = 0;
	_gl_info_loaded = false;
}

bool GL_Info::load() {
	const GLubyte *str = 0;
    char *tok = 0;

    // Get vendor string
    str = glGetString(GL_VENDOR);
    if(str) _vendor = reinterpret_cast<const char *>(str);
    else return false;

    // Get renderer string
    str = glGetString(GL_RENDERER);
    if(str) _renderer = reinterpret_cast<const char *>(str);
    else return false;

    // Get version string
    str = glGetString(GL_VERSION);
    if(str) _version = reinterpret_cast<const char *>(str);
    else return false;

    int major = 0;
	int minor = 0;

	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);

	_version_code = major * 100 + minor * 10;

    _gl_info_loaded = true;
    return true;
}

uint GL_Info::versionCode() const {
	return _version_code;
}

void GL_Info::printInfo() {
	if (!_gl_info_loaded) {
		fprintf(stdout, "Incomplete OpenGL information:\n");
	} else {
		fprintf(stdout, "OpenGL information:\n");
	}

	fprintf(stdout, "-------------------------------------------\n");
	fprintf(stdout, "OpenGL vendor is: %s\n", _vendor.c_str());
	fprintf(stdout, "OpenGL renderer is: %s\n", _renderer.c_str());
	fprintf(stdout, "OpenGL version is: %s\n", _version.c_str());
	fprintf(stdout, "-------------------------------------------\n");
}

} // namespace