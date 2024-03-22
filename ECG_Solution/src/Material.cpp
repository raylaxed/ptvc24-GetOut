/*
* Copyright 2020 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/
#include "Material.h"

/* --------------------------------------------- */
// Base material
/* --------------------------------------------- */

Material::Material(std::shared_ptr<Shader> shader, glm::vec3 materialCoefficients, float alpha)
	: _shader(shader), _materialCoefficients(materialCoefficients), _alpha(alpha)
{
}

Material::Material(std::shared_ptr<Shader> shader)
	: _shader(shader)
{
}

Material::~Material()
{
}

Shader* Material::getShader()
{
	return _shader.get();
}

void Material::setUniforms()
{

		_shader->setUniform("materialCoefficients", _materialCoefficients);
		_shader->setUniform("specularAlpha", _alpha);

}

/* --------------------------------------------- */
// Texture material
/* --------------------------------------------- */

TextureMaterial::TextureMaterial(std::shared_ptr<Shader> shader, glm::vec3 materialCoefficients, float alpha, std::shared_ptr<Texture> diffuseTexture)
	: Material(shader, materialCoefficients, alpha), _diffuseTexture(diffuseTexture)
{

	_baseColor = nullptr;
	_ambientOcclusion = nullptr;
	_metallic = nullptr;
	_normal = nullptr;
	_roughness = nullptr;

}


TextureMaterial::TextureMaterial(std::shared_ptr<Shader> shader, std::shared_ptr<Texture> baseColor,
								std::shared_ptr<Texture> ambientOcclusion, std::shared_ptr<Texture> metallic,
								std::shared_ptr<Texture> normal, std::shared_ptr<Texture> roughness)
								: Material(shader), _baseColor(baseColor), _ambientOcclusion(ambientOcclusion), _metallic(metallic),
								_normal(normal), _roughness(roughness)
{

	_diffuseTexture = nullptr;

}

TextureMaterial::~TextureMaterial()
{
}

void TextureMaterial::setUniforms()
{

	if (_diffuseTexture != nullptr) {
		Material::setUniforms();

		_diffuseTexture->bind(0);
		_shader->setUniform("diffuseTexture", 0);
	}
	else {

		_baseColor -> bind(0);
		
		_shader->setUniform("baseColorTex", 0);

		_ambientOcclusion->bind(1);
		_shader->setUniform("aoTex", 1);

		_metallic->bind(2);
		_shader->setUniform("metallicTex", 2);

		_normal->bind(3);
		_shader->setUniform("normalTex", 3);

		_roughness->bind(4);
		_shader->setUniform("roughnessTex", 4);
	
	
	}
}
