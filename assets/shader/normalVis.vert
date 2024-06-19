#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

out VertexData {
    vec3 normal_world;
    vec4 position_world;
} vert;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat3 normalMatrix;
uniform float u_time;

float displacement(vec3 position) {
    float s_contrib = 0.5 * sin(position.x * 2.5 + 3.0 * u_time);
    float t_contrib = 0.3 * cos(position.y * 1.1 + 2.0 * u_time);
    return s_contrib * t_contrib;
}

vec3 orthogonal(vec3 v) {
    return normalize(abs(v.x) > abs(v.z) ? vec3(-v.y, v.x, 0.0) : vec3(0.0, -v.z, v.y));
}

void main() {
    vec3 dispPosition = position + normal * displacement(position);

    vec3 tangent = orthogonal(normal);
    vec3 bitangent = normalize(cross(normal, tangent));
    vec3 neighbour1 = position + tangent * 0.01;
    vec3 neighbour2 = position + bitangent * 0.01;
    vec3 dispN1 = vec3(neighbour1.x, displacement(neighbour1), neighbour1.z);
    vec3 dispN2 = vec3(neighbour2.x, displacement(neighbour2), neighbour2.z);

    vec3 dispTangent = dispN1 - dispPosition;
    vec3 dispBitangent = dispN2 - dispPosition;
    vec3 displacedNormal = normalize(cross(dispTangent, dispBitangent));

    vert.normal_world = normalMatrix * displacedNormal;
    vert.position_world = modelMatrix * vec4(dispPosition, 1.0);
    gl_Position = projMatrix * viewMatrix * vert.position_world;
}
