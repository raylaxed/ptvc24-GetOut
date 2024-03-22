#include "Text.h"


Text::Text(std::string text, glm::vec2 position, float scale, glm::vec3 color, std::map<GLchar, Character>& characters,Shader& shader)
	: _text(text), _position(position),_scale(scale), _color(color), _characters(characters), _shader(&shader)
{

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);
	
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

Text::~Text() {

	glDeleteVertexArrays(1, &_vao);
}

void Text::setText(std::string newText) {

	_text = newText;

}

void Text::setColor(glm::vec3 color) {

	_color = color;

}


void Text::drawText(bool isDashCooldown) {


	int xForCalc = _position.x;
	// activate corresponding render state	
	_shader->use();
	glUniform3f(_shader->getUni("textColor"), _color.x, _color.y, _color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(_vao);

	// iterate through all characters
	std::string::const_iterator chars;
	for (chars = _text.begin(); chars != _text.end(); chars++)
	{

		Character ch = _characters[*chars];

		float xpos = xForCalc + ch.Bearing.x * _scale;
		float ypos = _position.y - (ch.Size.y - ch.Bearing.y) * _scale;
		float w, h;
		if (isDashCooldown) {
			w = ch.Size.x * _scale * 2.5;
			h = ch.Size.y * _scale * 0.5f;
		}
		else {

			w = ch.Size.x * _scale;
			h = ch.Size.y * _scale;
		}

		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		xForCalc += (ch.Advance >> 6) * _scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

}