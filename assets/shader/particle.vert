#version 430 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec4 positionAndSize;
layout (location = 2) in vec4 color;

out vec2 TexCoords;
out vec4 ParticleColor;

uniform vec3 camRight;
uniform vec3 camUp;
uniform mat4 viewProjection;


void main()
{
    float scale = positionAndSize.w;
    vec3 position = positionAndSize.xyz;
    vec3 vertexPosition_world = position + camRight * vertex.x * scale
                                         + camUp * vertex.y * scale;

    gl_Position = viewProjection * vec4(vertexPosition_world, 1.0f);

    TexCoords = vertex.xy + vec2(0.5f,0.5f);
    ParticleColor = color;

}