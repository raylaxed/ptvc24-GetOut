#pragma once

#include <glm\glm.hpp>

class Bloom
{
public:
	Bloom(int srcWidth, int srcHeight);
	~Bloom();

	void Blur();

private:
	int SCR_WIDTH;
	int SCR_HEIGHT;

};

Bloom::~Bloom()
{
}

inline void Bloom::Blur()
{
}
