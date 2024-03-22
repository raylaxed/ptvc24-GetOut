#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

#include <string>
#include <vector>
using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct ModelTexture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {

public:
    // mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<ModelTexture>      textures;
    unsigned int VAO;

    
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<ModelTexture> textures);

    ~Mesh();

    // used to render the mesh
    void Draw(Shader &shader);

private:
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh();
};