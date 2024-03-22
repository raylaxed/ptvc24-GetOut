#pragma once

#include <vector>
#include <memory>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <GL\glew.h>
#include "Material.h"
#include "Shader.h"


class Text {


private:

	GLuint _vao;

	GLuint _vbo;

	std::string _text;

	glm::vec2 _position;

	float _scale;

	glm::vec3 _color;

	std::map<GLchar, Character> _characters;

	Shader* _shader;

public:

	Text(std::string text, glm::vec2 position, float scale, glm::vec3 color, std::map<GLchar, Character>& characters, Shader& shader);
	
	~Text();

	void setText(std::string newText);
	void setColor(glm::vec3 color);

	void drawText(bool isDashCooldown = false);

};