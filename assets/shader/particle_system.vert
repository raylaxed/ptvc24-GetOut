#version 430 core

// Input attributes
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec4 particlePositionSize;
layout(location = 2) in vec4 particleColor;

// Uniforms
uniform mat4 viewProjectionMatrix;
uniform mat4 view;
uniform mat4 projection;

// Output to fragment shader
out vec4 color;

void main() {
    // Calculate the particle's position in world space
    vec3 particlePosition = particlePositionSize.xyz;
    float particleSize = particlePositionSize.w;

    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);

    
    vec3 vertexPosition = particlePosition 
                        + cameraRight * vertexPosition_modelspace.x * particleSize 
                        + cameraUp * vertexPosition_modelspace.y * particleSize;

    gl_Position = projection * view * vec4(vertexPosition, 1.0);


    // Offset the vertex position by the particle's position and size
    //vec4 vertexPosition_worldspace = vec4(particlePosition + vertexPosition_modelspace * particleSize, 1.0);

    // Apply the view-projection matrix to get the final vertex position in clip space
    //gl_Position = viewProjectionMatrix * vertexPosition_worldspace;

    // Pass the color to the fragment shader
    color = particleColor;
}
