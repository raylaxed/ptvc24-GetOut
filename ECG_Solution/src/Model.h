


#ifndef MODEL_H
#define MODEL_H


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

class Model
{
public:
    // model data 
    vector<ModelTexture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh>    meshes;
    string directory;
    Shader* _shader;
    
    bool gammaCorrection;

    // constructor, expects a filepath to a 3D model.
    Model(string const& path, glm::mat4 modelMatrix,  Shader& shader, bool gamma = false);

    ~Model();

    // draws the model, and thus all its meshes
    void Draw(glm::mat4 model);
    void Draw(Shader& shader);
    void Model::Draw(float time, glm::mat4 model);
    void setModel(glm::mat4 model);


    void resetModelMatrix();

    glm::mat4 getModel();

private:

    glm::mat4 _modelMatrix;

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const& path);
    

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene);
   

    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<ModelTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
    
};



#endif

