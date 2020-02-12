#ifndef __FASTA_MESH_H__
#define __FASTA_MESH_H__

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "fasta_lib/shader.h"

namespace fst {

struct FstVertex {
    glm::vec3 Position;		// Position
    glm::vec3 Normal;		// Normal
    glm::vec2 TexCoords;	// TexCoords
    glm::vec3 Tangent;		// Tangent
    glm::vec3 Bitangent;	// Bitangent
};

struct FstTexture {
    GLuint id;
    std::string type;
    std::string path;
};

class FstMesh {
public:
	FstMesh(std::vector<FstVertex> vertices, std::vector<GLuint> indices, std::vector<FstTexture> textures);

	// Render mesh
	void Draw(FstShader shader);

private:
	/*  Mesh Data  */
    std::vector<FstVertex> vertices;
    std::vector<GLuint> indices;
    std::vector<FstTexture> textures;
    GLuint VAO;

    /*  Render Data  */
    GLuint VBO, EBO;

	/*  Functions    */
    // Initializes all the buffer objects/arrays
    void setupMesh();
};

} // namespace

#endif // __FASTA_MESH_H__