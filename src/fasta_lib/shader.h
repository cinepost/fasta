#ifndef __FASTA_SHADER_H__
#define __FASTA_SHADER_H__

#include <stdlib.h>
#include <string.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>

class FstShader {
    public:
        // Constructor generates the shader on the fly
        FstShader();
        bool load(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath = NULL);
        
        // Uses the current shader
        void use();
        GLuint getProgram();

        // utils
        static std::string getShaderLog(GLuint obj);
        static std::string getProgramLog(GLuint obj);

    private:
        GLuint _program;
        bool loaded;
};

#endif // __FASTA_SHADER_H__