#pragma once

#include "ParticleSystem.h"




ParticleSystem::ParticleSystem(std::shared_ptr<Shader>& shader, Camera& cam, float offsetFactor, float size, unsigned int amount, glm::vec3 position)
    : shader(shader), _camera(&cam), _amount(amount), _offsetFactor(offsetFactor), _size(size), _position(position) {

    this->init();
    _particle_color_data = new GLubyte[_amount * 4];
    _particle_position_data = new GLfloat[_amount * 4];
}


void ParticleSystem::init() {


    glGenBuffers(1, &_billboard_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, _billboard_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // The VBO containing the positions and sizes of the particles

    glGenBuffers(1, &_particles_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, _particles_position_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, _amount * 3 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

    // The VBO containing the colors of the particles

    glGenBuffers(1, &_particles_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, _particles_color_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, _amount * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);


    for (int i = 0; i < this->_amount; i++)
    {
        this->_particles.push_back(Particle());
    }

}


void ParticleSystem::Update(float deltaTime, unsigned int newParticles,
    glm::vec3 objectPosition)
{
    for (int i = 0; i < newParticles; ++i)
    {
        int unusedParticle = firstUnusedParticle();
        respawnParticle(_particles[unusedParticle], objectPosition);
    }

    _pCount = 0;

    for (int i = 0; i < _amount; ++i)
    {
        Particle& tmp = _particles[i];

        if (tmp._life > 0.0f) {

            tmp._life -= deltaTime;

            if (tmp._life > 0.0f)
            {
                tmp._position += tmp._velocity * deltaTime;

                if ((float)tmp.r != 0) {
                    tmp.r -= (deltaTime / 1000.f);
                }
                if ((float)tmp.g != 0) {
                    tmp.g -= (deltaTime / 100.f);
                }
                if ((float)tmp.b != 0) {
                    tmp.b += (deltaTime / 10.f);
                }
                tmp.a -= (deltaTime / 2000.f);

                tmp.camDistance = glm::length2(tmp._position - _camera->getPosition());

                // Fill the GPU buffer
                _particle_position_data[4 * _pCount + 0] = tmp._position.x;
                _particle_position_data[4 * _pCount + 1] = tmp._position.y;
                _particle_position_data[4 * _pCount + 2] = tmp._position.z;
                _particle_position_data[4 * _pCount + 3] = tmp._size;


                _particle_color_data[4 * _pCount + 0] = tmp.r;
                _particle_color_data[4 * _pCount + 1] = tmp.g;
                _particle_color_data[4 * _pCount + 2] = tmp.b;
                _particle_color_data[4 * _pCount + 3] = tmp.a;

            }
            else {

                tmp.camDistance = -1;
            }
            _pCount++;
        }
    }
    SortParticles();
}


unsigned int ParticleSystem::firstUnusedParticle()
{

    for (int i = lastUsedParticle; i < _amount; ++i) {
        if (_particles[i]._life <= 0.0f)
        {
            lastUsedParticle = i;
            return i;
        }
    }

    for (int i = 0; i < lastUsedParticle; ++i) {
        if (_particles[i]._life <= 0.0f)
        {
            lastUsedParticle = i;
            return i;
        }
    }

    lastUsedParticle = 0;
    return 0;

}

void ParticleSystem::respawnParticle(Particle& particle, glm::vec3 objectPosition)
{

    std::random_device rd;
    std::uniform_real_distribution<double> distMinus(-1.0, 1.0);
    std::uniform_real_distribution<double> distSize(0, _size);

    glm::vec3 offset = glm::vec3(
        distMinus(rd),
        distMinus(rd),
        distMinus(rd)
    );

    particle._position = objectPosition + _position + offset * _offsetFactor;

    glm::vec3 mainDirection = glm::vec3(0.0f, 2.0f, 0.0f);
    glm::vec3 randomDirection = glm::vec3(
        distMinus(rd) / 2.0f,
        distMinus(rd) / 2.0f,
        distMinus(rd) / 2.0f
    );

    float spread = 2.f;

    particle._life = 0.5f;
    particle._velocity = mainDirection + randomDirection * spread;
    particle._size = distSize(rd);

    particle.r = 255;
    particle.g = 130;
    particle.b = 1;
    particle.a = 150;

}

void ParticleSystem::SortParticles() {
    std::sort(_particles.begin(), _particles.end());
}

void ParticleSystem::Draw()
{

    glBindBuffer(GL_ARRAY_BUFFER, _particles_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, _amount * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf.
    glBufferSubData(GL_ARRAY_BUFFER, 0, _pCount * sizeof(GLfloat) * 4, _particle_position_data);

    glBindBuffer(GL_ARRAY_BUFFER, _particles_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, _amount * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf.
    glBufferSubData(GL_ARRAY_BUFFER, 0, _pCount * sizeof(GLfloat) * 4, _particle_color_data);


    shader->use();
    shader->setUniform("viewProjectionMatrix", _camera->getProjectionMatrix() * _camera->GetViewMatrix());
    shader->setUniform("view", _camera->GetViewMatrix());
    shader->setUniform("projection", _camera->getProjectionMatrix());

    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // 1st attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, _billboard_vertex_buffer);
    glVertexAttribPointer(
        0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // 2nd attribute buffer : positions of particles' centers
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, _particles_position_buffer);
    glVertexAttribPointer(
        1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        4,                                // size : x + y + z + size => 4
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

    // 3rd attribute buffer : particles' colors
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, _particles_color_buffer);
    glVertexAttribPointer(
        2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        4,                                // size : r + g + b + a => 4
        GL_UNSIGNED_BYTE,                 // type
        GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
        0,                                // stride
        (void*)0                          // array buffer offset
    );

    // These functions are specific to glDrawArrays*Instanced*.
    // The first parameter is the attribute buffer we're talking about.
    // The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
    // http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
    glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
    glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
    glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

    // Draw the particles !
    // This draws many times a small triangle_strip (which looks like a quad).
    // This is equivalent to :
    // for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
    // but faster.
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, _pCount);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}


void ParticleSystem::DestroyParticleSystem()
{
    glDeleteBuffers(1, &_particles_color_buffer);
    glDeleteBuffers(1, &_particles_position_buffer);
    glDeleteBuffers(1, &_billboard_vertex_buffer);
    glDeleteVertexArrays(1, &VertexArrayID);
}
