#pragma once

#include <glm\glm.hpp>
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include <vector>
#include <glm/gtx/norm.hpp>
#include <random>



struct Particle {

	glm::vec3 _position;
	glm::vec3 _velocity;
	glm::vec4 _color;
	float _life;
	float _size;
	float camDistance;
	unsigned char r, g, b, a;

	Particle() : _position(0.0f), _velocity(0.0f), _color(1.0f), _life(0.0f) {}

	bool operator<(const Particle& that) const {
		return this->camDistance > that.camDistance;
	}
};

class ParticleSystem
{

private:
	std::vector<Particle> _particles;
	float _offsetFactor;
	float _size;
	unsigned int _amount;
	int _pCount = 0;
	unsigned int lastUsedParticle = 0;
	glm::vec3 _position;

	const GLfloat g_vertex_buffer_data[12] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f
	};
	GLfloat* _particle_position_data;
	GLubyte* _particle_color_data;

	std::shared_ptr<Shader> shader;
	Camera* _camera;
	unsigned int _vao;
	GLuint _billboard_vertex_buffer;
	GLuint _particles_position_buffer;
	GLuint _particles_color_buffer;
	GLuint VertexArrayID;

	void init();
	unsigned int firstUnusedParticle();
	void respawnParticle(Particle& particle, glm::vec3 objectPosition);

public:
	ParticleSystem(std::shared_ptr<Shader>& shader, Camera& cam, float offsetFactor, float size, unsigned int amount, glm::vec3 position = glm::vec3(0.f));

	void Update(float deltaTime, unsigned int newParticles, glm::vec3 objectPosition = glm::vec3(0.f));
	void Draw();
	void ParticleSystem::SortParticles();
	void DestroyParticleSystem();
};