#include <vector>
#include "fasta_lib/shader.h"

FstShader::FstShader(){
    loaded = false;
}

bool FstShader::load(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath)
{
    // 1. Retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;

    // ensures ifstream objects can throw exceptions:
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    
    // red env variable XN_SHADERS_HOME
    char *pPath;
    pPath = getenv("XN_SHADERS_HOME");
    if (pPath==NULL)
        pPath = "/home/max/dev/xenon/src/shaders/";

    char fullVertexPath[256];
    char fullFragmentPath[256];

    strncpy(fullVertexPath, pPath, sizeof(fullVertexPath));
    strncat(fullVertexPath, vertexPath, sizeof(fullVertexPath));

    strncpy(fullFragmentPath, pPath, sizeof(fullFragmentPath));
    strncat(fullFragmentPath, fragmentPath, sizeof(fullFragmentPath));

    try {
        // Open files
        vShaderFile.open(fullVertexPath);
        fShaderFile.open(fullFragmentPath);
        std::stringstream vShaderStream, fShaderStream;

        // Read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();		

        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        
        // Convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();			
        
        // If geometry shader path is present, also load a geometry shader
        if(geometryPath != NULL)
        {
            gShaderFile.open(geometryPath);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::ifstream::failure e){    
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        return false;
    }

    const GLchar* vShaderCode = vertexCode.c_str();
    const GLchar* fShaderCode = fragmentCode.c_str();

    // 2. Compile shaders
    GLuint vertex, fragment;
    GLint success;
    GLchar infoLog[512];

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if ( success == GL_FALSE) {
        return false;
    }

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if ( success == GL_FALSE ) {
        return false;
    }
    
    // If geometry shader is given, compile geometry shader
    GLuint geometry;
    if(geometryPath != NULL)
    {
        const GLchar * gShaderCode = geometryCode.c_str();
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
        if ( success == GL_FALSE ) {
            return false;
        }
    }

    // Shader _program
    this->_program = glCreateProgram();
    glAttachShader(this->_program, vertex);
    glAttachShader(this->_program, fragment);

    if(geometryPath != NULL)
        glAttachShader(this->_program, geometry);

    glLinkProgram(this->_program);
    glGetProgramiv(this->_program, GL_LINK_STATUS, &success);
    if ( success == GL_FALSE ) {
        return false;
    }

    // Delete the shaders as they're linked into our _program now and no longer necessery
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    if(geometryPath != NULL)
        glDeleteShader(geometry);

    loaded = true;
    return loaded;
}
    
void FstShader::use(){
    glUseProgram(this->_program);
}

GLuint FstShader::getProgram(){
    return _program;
}

// static
std::string FstShader::getShaderLog(GLuint shader_obj){
    int log_length = 0;
    glGetShaderiv(shader_obj, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length > 0) {
        std::vector<GLchar> log(log_length);
        glGetShaderInfoLog(shader_obj, log_length, NULL,  &log[0]);
        return std::string(log.begin(), log.end());
    }
    return "";
}


// static
std::string FstShader::getProgramLog(GLuint program_obj) {
    int log_length = 0;

    glGetProgramiv(program_obj, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length > 0) {
        std::vector<GLchar> log(log_length);
        glGetShaderInfoLog(program_obj, log_length, NULL, &log[0]);
        return std::string(log.begin(), log.end());
    }
    return "";
}